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
      if p < tmp_glb_min:
        tmp_glb_min = p

  if tmp_glb_min < global_min:
    global_min = tmp_glb_min
```

The algorithm execution consists of a number of iterations determined by a certain termination condition, for the sake of simplicity this is a integer provided by the user.
Because of the constant and minimum contribution to the performance, the parallelization of the work needed to compute the initial state of the system has not been addressed in this report nor in the implementation.

Identifying the dependencies allows a **functional deconstruction** of the body of the loop:

- the update of the *"minimum particles"* depends on the value of the user provided function in the point
- the value depends on the current position of the particle
- the position depends on the current velocity
- the velocity of a particle depends on the local and global minimum

Each iteration computes a candidate to be the global minimum of a certain real function; the computation depends on the result of the previous one, so a **barrier** must exist to insure coherency.
As in the sequential pseudo-code, where the global minimum is chosen after all the particles have contributed to compute the state.

Given these dependencies, it's possible to benefit of a **pipeline introduction**.

The vector nature of the problem, given by the position and the speed of an object in a multidimensional space, is prone to the use of **vectorization** to parallelize the computation in independent dimensions.
For the vectorization to be effective the operations to limit the boundaries of the solution space could be limited by the ALU's architecture, for example the modulo operator useful to provide a toroidal space couldn't be available, so this choice has to be experimental evaluated.

Whilst the external loop can't be parallelized because of the dependency between consequent states, the internal one is fully exploitable using a farm because of the independent nature of the particles.
Once all the particles have been computed by the farm workers, the global minimum can be updated.

```
Pipe (
  Farm ( 
    Pipe (
      velocity,
      position,
      f,
      min
    )
  ),
  min
)
```

![Parallel pattern structure](img/structure.png)

# Performance model

The expected sequential time is:

$$
T_{seq} = 2 ( T_{vel} + T_{pos} ) + T_f + T_{loc}
$$

After the vectorization and the pipeline introduction the latency and the service time should vary like this:

$$
L = T_{vel} + T_{pos} + T_f + T_{loc}
$$
$$
T_s = max ( T_{vel} , T_{pos} , T_f , T_{loc} )
$$

The farm introduction with *nw* workers should be able to increment the inner loop metrics:

$$
L = \frac{T_{vel} + T_{pos} + T_f + T_{loc}}{nw}
$$
$$
T_s = \frac{max ( T_{vel} , T_{pos} , T_f , T_{loc} )}{nw}
$$

# Implementation details

# Experimental results
