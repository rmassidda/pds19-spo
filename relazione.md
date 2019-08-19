# Parallel architecture design

```python
while not termination:
  tmp_glb_min = INF
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

The algorithm execution consists of a number of iterations determined by a certain termination condition, for the sake of simplicity this is a integer provided by the user.
Because of the constant and minimum contribution to the performance, the parallelization of the work needed to compute the initial state of the system has not been addressed in this report nor in the implementation.

## General considerations

Each iteration computes a candidate to be the global minimum of a certain real function; the computation depends on the result of the previous states, so a **barrier** must exist to insure soundness.

The vector nature of the problem, given by the position and the speed of an object in a multidimensional space, is prone to the use of **vectorization** to parallelize the computation in independent dimensions.

## Map+Reduce

A possible approach is to combine the **Map** and the **Reduce** patterns to parallelize the computation of a state.

The function given to the *map* should take a particle, update its internal state as a side-effect and return a reference to the local minimum.

$$
f: \textrm{particle} \rightarrow \textrm{result}
$$

Given that the global minimum is always the minimum of the local minimums of all the particles, it's possible to *reduce* the result from the updated particles.

$$
min: \textrm{result} \times \textrm{result} \rightarrow \textrm{result}
$$

![](img/mapreduce.png)

## Pipeline

Identifying the dependencies allows a **functional deconstruction** of the body of the loop:

- the update of the *"minimum particles"* depends on the value of the user provided function in the point
- the value depends on the current position of the particle
- the position depends on the current velocity
- the velocity of a particle depends on the local and global minimum

Given this functional dependencies, it could be possible to benefit of a **pipeline introduction** using a *scatter* to generate a stream from a collection.

![](img/pipe.png)

Since the user provided $f$ function can be arbitrarily complex its service time could easily become the bottleneck of the pipeline.
The proof of this intuition and the evidence of other bottlenecks could emerge during the *profiling* phase after the implementation.
The performance could benefit so of a **farm introduction**.

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

In the pipeline solution the estimated service time leads to the completion time with an high $n$ value.

$$
T_s = \max ( T_v , T_p , T_f , T_m )
$$

$$
T_{\textrm{Pipe}} \approx n * T_{s} = n * \max ( T_v , T_p , T_f , T_m )
$$

After the farm introduction to avoid the bottleneck the estimated completion time becomes:

$$
T_{\textrm{PipeFarm}} \approx n * \max ( T_v , T_p , \frac{T_f}{n_w} , T_m )
$$

Assuming that an high number of particles is involved in the computation and that the $f$ complexity requires a significant time both the PipeFarm and the MapReduce solutions are theoretically equivalent.
The communication needed in between the stages in the pipeline solution and the effort needed to generate a stream from a collection could produce a consistent overhead not present in the data parallel solution, so the data parallel solution is favorable.

# Implementation details

# Experimental results
