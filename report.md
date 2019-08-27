# Parallel architecture design

```python
while not termination:
  for p in particles:
    p.update_velocity ( global_min )
    p.update_position ()
    p.update_value ( user_function )
    if p < p.local_min:
      p.local_min = p

  for p in particles:
    if p.local_min < global_min:
      global_min = p.local_min
```

The algorithm execution consists of several iterations determined by a certain termination condition, for the sake of simplicity this number consists of an integer provided by the user.
Because of the constant and minimum contribution to the performance, the parallelization of the work needed to compute the initial state of the system has not been addressed in the implementation and this report.

## General considerations

Each iteration computes a candidate to be the global minimum of a certain real function; the computation depends on the result of the previous states, so a **barrier** between subsequent iterations must exist to ensure soundness.

The computation of the movement and the value of the function in a certain point is *embarrassing parallel* because of the independence of the particles. 

The vector nature of the problem, given by the position and the speed of an object in a multidimensional space, is prone to the use of **vectorization** to parallelize the computation in independent dimensions.

## Data parallel solution

A possible approach is to combine the **map** and the **reduce** high-order functions to parallelize the computation of a state.

The elemental function given to the *map* should take a particle, update its internal state and return a reference to the local minimum.

$$
f: \textrm{particle} \rightarrow \textrm{result}
$$

Given that the global minimum is always the minimum of the local minimums of all the particles, it's possible to *reduce* the result from the updated particles using the following *combiner function*.

$$
\min: \textrm{result} \times \textrm{result} \rightarrow \textrm{result}
$$

A two-phase reduction with a local reduction of a chunk and a global reduction for all the results is feasible because of the benefits of fusing map and reduce; this solution doesn't present *precision* problems because of the nature of the minimization operator, so it's possible to avoid a more complex and expensive tree-based reduction.

![](img/mapreduce.png)

## Stream parallel solution

Identifying the data dependencies allows a *functional deconstruction* of the body of the loop:

- the update of the local minimum of a particle depends on the result of the user-provided function
- the computed value depends on the current position of the particle
- the position depends on the current velocity
- the velocity of a particle depends on the local and global minimum

Given these functional dependencies, it could be possible to benefit from a *pipeline introduction* using a first stage capable to generate a stream from a collection.

![](img/pipe.png)

Since the user-provided $f$ function can be arbitrarily complex its service time could easily become the bottleneck of the pipeline.
The proof of this intuition and the evidence of other bottlenecks could emerge during the *profiling* phase after the implementation.
The performance could benefit so from a farm introduction.

![](img/farm.png)

# Performance model

Given $n$ as the number of particles and $m$ the number of total iterations to compute, the expected sequential time is:

$$
T_{\textrm{seq}} = m * n ( T_{v} + T_{p} + T_{f} + 2 T_{m} )
$$

As previously discussed, the $m$ factor can't be exploited via parallelization, so the performance analysis could be done referencing only the internal loop used to compute a certain state.

In the sequential case this takes:

$$
T_{\textrm{seq}} = n ( T_{v} + T_{p} + T_{f} + 2 T_{m} )
$$

## Comparison

The performances of the proposed solutions are easily comparable in terms of completion time for each iteration.

$$
T_{\textrm{map-reduce}} = \frac{n}{n_w} ( T_v + T_p + T_f + 2 T_m ) + n_w * T_m 
$$

In the pipeline solution, the estimated service time leads to the completion time for a high $n$ value.

$$
T_s = \max ( T_v , T_p , T_f , T_m )
$$

$$
T_{\textrm{Pipe}} \approx n * T_{s} = n * \max ( T_v , T_p , T_f , T_m )
$$

After the farm introduction to avoid the $f$ bottleneck the estimated completion time becomes:

$$
T_{\textrm{PipeFarm}} \approx n * \max ( T_v , T_p , \frac{T_f}{n_w} , T_m )
$$

Assuming that a high number of particles is involved in the computation and that the $f$ complexity requires significant time, both the PipeFarm and the map-reduce solutions are theoretically equivalent.

However it has to be considered that the communication needed between the stages in the pipeline solution and the effort needed to generate a stream from a collection, could produce a consistent overhead not present in the data-parallel solution
Furthermore the possible adjustments to mitigate the pipeline overhead problem, as the fusion of the faster stages, would lead to a normal form stream-parallel pattern that resembles the data-parallel proposed solution except for the requirement to generate a stream.

For these reasons, the data-parallel solution is favorable and it's been chosen for the implementation phase.

# Implementation details

The problem is modeled using two data structures described in the `spo.hpp` file:

- `result_t` stores the coordinates of a certain point and the value of the user-provided function in the same point.
- `particle_t` keeps track of the velocity of a particle other than the results obtained in the current and the local minimum positions.

The set of particles is stored in an array of structures.
Because the same worker needs to read and write different members of the same particle there is no use for a structures-of-array solution that, without evident performance advantages, only breaks data encapsulation.

To update the particle set is used a lambda function that sequentially applies all the operations needed to compute the current velocity, position, value, and local minimum.
The function chosen to be minimized is the following:

$$
f(x,y) = \int_x^y \sin(t) dt
$$

The function presents a unique global minimum in the interval $x,y \in \left[ 0, 4 \right]$ and is time complexity can be arbitrarily adjusted by setting the precision of the Riemann sum used to approximate it.

Given the report[^vectorization_report] generated by the GNU compiler using the `-fopt-info-vec-all` not all the for loops can be vectorized:

- The update of the velocity can't be vectorized due to the random number generation; even storing in an external vector the result of the random generation before the loop doesn't produce vectorization because of the "loop costings not worthwhile", as g++ notified.
- No issues appear in the vectorization of the computation of the position, but the control of the boundaries of the space isn't vectorizable due to internal loops without a fixed number of iterations.

The implementation of the combination of map and reduce is in the `mapreduce.hpp` file.
The class *MapReduce* uses a thread pool initialized in the construction phase and reused in all the subsequent calls, this behavior avoids the expensive creation of one or more threads in each iteration.

To notify the start of a new iteration or the termination of the procedure, the main thread communicates to the workers via a set of queues where to push the values `GO` and `EOS`.
Each worker is responsible for the computation of a certain fixed size chunk in the input collection of particles, for each element it applies the provided elemental function and computes a local reduction, the result of this local reduction is pushed inside a queue where the main thread pops from to conclude the reduce phase.

An alternative approach is to use a unique queue, used by the main thread to push the coordinates of a chunk to the workers.
This solution is applicable via git using the command `git apply queue.patch`.

By default the MapReduce constructor forces the correlation between threads and cores, but this can be demanded by the user using an optional boolean argument when creating the object that provokes the constructor to call `pthread_setaffinity_np` for each thread.
 
`main.cpp` is used to merge the business logic and the parallelization mechanism, depending on the $n_w$ provided it can sequentially compute the swarm particle optimization ( $n_w = 0$ ), or in parallel using the map-reduce or the FastFlow approach.

The FastFlow solution uses the `ParallelForReduce` class to instantiate $n_w$ threads and reuse them to map the update function to all the particles and subsequently to obtain the global minimum via reduction of the local minimums.
As in the C++ threads solution, the map and the local reduction are executed sequentially.

Both the queues used for the communication between threads and the class used to monitor times are the "standard" ones used for the PDS course.
Those are respectively in the `queue.hpp` and `utimer.hpp` files.

After setting the variable `$FF_ROOT` to the FastFlow current path in the Makefile, the project is compilable using the command: `make spo`.
The utility `autotime.sh` can be used to keep track of the times of an experiment executing the `spo` software with an increasing number of workers and storing the results in a dedicated folder.
Without adding redundant information to this report is possible to see the required argument list by calling both `spo` and `autotime.sh` without any argument.

# Experimental results

The performance metrics are directly influenced by the three factors:

- The number of particles involved in the optimization process
- The precision of the Riemann sum that increases the $t_f$ time and consequently the time needed to update the state of a particle.
- The correlation between threads and cores

Different experiments have been computed on the Xeon machine available for test\textsuperscript{(\ref{fig:experiments})}, both with the core's affinity enabled and without.

\begin{figure}[!htb]

\begin{tabular}{l*{6}{c}r}
Codename          & Particles & Iterations & Precision & Max Workers \\
\hline
big\_p10 &100000 &100 &0.1 &128 \\ 
little\_p10 &1000 &100 &0.1 &128 \\ 
little\_p10000 &1000 &100 &0.0001 &128 \\
\end{tabular}

\caption{Experiments}\label{fig:experiments}
\end{figure}

A great effort to the speedup trend is given by the dimension of the particle set as is visible in figure\textsuperscript{(\ref{fig:sizespeed})}, where the two experiments share the same precision but the one with a biggest set has a significant smaller particle set.
The last two experiments used a little particle set ($10^3$) to compute the minimum of the definite integral.
The time needed to update the state of a single particle, influenced by the Riemann sum precision, contributes consistently to improve the speedup trend as is visible in figure\textsuperscript{(\ref{fig:precspeed})}.

As a general trend the custom solution outperforms the `ParallelForReduce` method offered by FastFlow in all the different experiments, this is visible in figures\textsuperscript{(\ref{fig:efficiency})} and\textsuperscript{(\ref{fig:speedup})}.
All the raw data from the experiments and the plots that represents the comparison of FastFlow and the custom solutions in terms of scalability, speedup and efficiency are stored in the `./experiment/` folder included in the archive.

Applying the patch to change the number of queues used doesn't dramatically change the performance, nonetheless the patched version seems to perform worse than the one with a queue per worker\textsuperscript{(\ref{fig:queues})}.
This should be caused by the high number of threads waiting on the same condition variable.

\begin{figure}[!htb]
  \includegraphics[width=\linewidth]{img/size_speedup.png}
  \caption{Same precision, different size}\label{fig:sizespeed}
  \includegraphics[width=\linewidth]{img/prec_speedup.png}
  \caption{Same size, different precision}\label{fig:precspeed}
\end{figure}

\begin{figure}[!htb]
\includegraphics[width=\linewidth]{img/sol_efficiency_size.png}
\includegraphics[width=\linewidth]{img/sol_efficiency_prec.png}
\caption{Efficiency comparison between FastFlow and the custom solution}
\label{fig:efficiency}
\end{figure}

\begin{figure}[!htb]
\includegraphics[width=\linewidth]{img/sol_speedup_size.png}
\includegraphics[width=\linewidth]{img/sol_speedup_prec.png}
\caption{Speedup comparison between FastFlow and the custom solution}
\label{fig:speedup}
\end{figure}

\begin{figure}[!htb]
  \includegraphics[width=\linewidth]{img/queue_number_scal.png}
  \caption{Same experiment, different number of queues}\label{fig:queues}
\end{figure}

[^vectorization_report]: A partial extract of the report can be found in the `vectorization.log` file.
