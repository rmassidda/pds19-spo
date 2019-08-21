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

The algorithm execution consists of a number of iterations determined by a certain termination condition, for the sake of simplicity this number consists in a integer provided by the user.
Because of the constant and minimum contribution to the performance, the parallelization of the work needed to compute the initial state of the system has not been addressed in this report nor in the implementation.

## General considerations

Each iteration computes a candidate to be the global minimum of a certain real function; the computation depends on the result of the previous states, so a **barrier** between subsequent iterations must exist to insure soundness.

The computation of the movement and the value of the function in a certain point is *embarrassing parallel* because of the independence of the particles, if the model included the possible impacts this wouldn't be true anymore.

The vector nature of the problem, given by the position and the speed of an object in a multidimensional space, is prone to the use of **vectorization** to parallelize the computation in independent dimensions.

## Data parallel solution

A possible approach is to combine the **map** and the **reduce** high-order functions to parallelize the computation of a state.

The elemental function given to the *map* should take a particle, update its internal state and return a reference to the local minimum.

$$
f: \textrm{particle} \rightarrow \textrm{result}
$$

Given that the global minimum is always the minimum of the local minimums of all the particles, it's possible to *reduce* the result from the updated particles using the following *combiner function*.

$$
min: \textrm{result} \times \textrm{result} \rightarrow \textrm{result}
$$

A two-phase reduction with a local reduction of a chunk and a global reduction for all the results is feasible because of the benefits of fusing map and reduce; this solution doesn't present *precision* problems because of the nature of the minimization operator, so it's possible to avoid a more complex and expensive tree-based reduction.

![](img/mapreduce.png)

## Stream parallel solution

Identifying the data dependencies allows a *functional deconstruction* of the body of the loop:

- the update of the local minimum of a particle depends on the result of the user provided function
- the computed value depends on the current position of the particle
- the position depends on the current velocity
- the velocity of a particle depends on the local and global minimum

Given this functional dependencies, it could be possible to benefit of a *pipeline introduction* using a first stage capable to generate a stream from a collection.

![](img/pipe.png)

Since the user provided $f$ function can be arbitrarily complex its service time could easily become the bottleneck of the pipeline.
The proof of this intuition and the evidence of other bottlenecks could emerge during the *profiling* phase after the implementation.
The performance could benefit so of a farm introduction.

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

The performance of the proposed solutions are easily comparable in terms of completion time for each iteration.

$$
T_{\textrm{MapReduce}} = \frac{n}{n_w} ( T_v + T_p + T_f + 2 T_m ) + n_w * T_m 
$$

In the pipeline solution the estimated service time leads to the completion time for a high $n$ value.

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

Assuming that an high number of particles is involved in the computation and that the $f$ complexity requires a significant time, both the PipeFarm and the MapReduce solutions are theoretically equivalent.
The communication needed in between the stages in the pipeline solution and the effort needed to generate a stream from a collection could produce a consistent overhead not present in the data parallel solution, so the data parallel solution is favorable and it's been chosen for the subsequent implementation.

# Implementation details

The problem is modelled using two data structures described in the `spo.hpp` file:

- `result_t` stores the coordinates of a certain point and the value of the user provided function in the same point.
- `particle_t` keeps track of the velocity of a particle other than the results obtained in the current and in the local minimum positions.

The set of particles is stored in an array of structures.
Because the same worker needs to read and write different members of the same particle there is no use for a structures of array solution that, without evident performance advantages, only breaks data encapsulation.

The function to update the particle sequentially applies all the operations needed to compute the current velocity, position, value and local minimum.
Given the report readable in the appendix generated by the GNU compiler using the `-fopt-info-vec-all` not all the for loops can be vectorized:

- The update of the velocity can't be vectorized due to the random number generation; even storing in an external vector the result of the random generation before the loop doesn't produce vectorization because of the "loop costings not worthwhile", as g++ notified.
- No issues appear in the vectorization of the computation of the position, but the control of the boundaries of the space isn't vectorizable due to the internal loops;
both the internal loops haven't got a fixed number of iterations, and each iteration depends on the precedent one, no these aren't vectorizable either.

The implementation of the combination of map and reduce is in the `mapreduce.hpp` file.
The class *MapReduce* uses a thread pool initialized in the construction phase and reused in all the subsequent calls, this behaviour avoids the expensive creation of one or more threads in each iteration.
Each thread is responsible for the computation of a certain fixed size chunk in the input collection of particles, for each element it applies the provided elemental function and computes a local reduction.
The result of this local reduction is pushed inside a queue where the main thread pops from to conclude the reduce phase.
 
The main is used to merge the business logic and the parallelization mechanism, depending on the $n_w$ provided it can compute the swarm particle optimization in a sequential way ( $n_w = 0$ ), using the mapreduce ($n_w > 0$) or using FastFlow ($n_w < 0$).

The FastFlow solution uses the `ParallelForReduce` class to instantiate an $n_w$ number of threads and reuse them to map the *update* function to all the particles and subsequently to obtain the global minimum via reduction of the local minimums.
As in the C++ threads solutions the map and the local reduction are executed sequentially.

Both the queues used for the communication between the main thread and the workers of the thread pool, and the class used to take times are the *standard* ones used for the PDS course.
Those are respectively in the `queue.hpp` and `utimer.hpp` files.

The project is compilable via a Makefile: `make spo`.

# Experimental results

# Appendix

## Vectorization report

```c
g++ -std=c++17 -Wall -I ~/fastflow -O3 -finline-functions -fopt-info-vec-all -o spo mapreduce.hpp spo.hpp utimer.hpp queue.hpp main.cpp -pthread
# Update velocity
main.cpp:93:24: missed: couldn't vectorize loop
main.cpp:93:24: missed: not vectorized: control flow in loop.

# Update position
main.cpp:104:24: missed: couldn't vectorize loop
main.cpp:104:24: missed: not vectorized: loop nest containing two or more consecutive inner loops cannot be vectorized
main.cpp:105:28: missed: couldn't vectorize loop
main.cpp:105:28: missed: not vectorized: number of iterations cannot be computed.
main.cpp:108:28: missed: couldn't vectorize loop
main.cpp:108:28: missed: not vectorized: number of iterations cannot be computed.
```

