# Parallel architecture design

The algorithm execution consists of a number of iterations determined by a certain termination condition, for the sake of simplicity this is a integer provided by the user.
Each iteration depends on the result of the previous one, so a **barrier** must exist to insure that the state at a certain step has been fully computed.

In pseudo-code the flow inside an iteration is the following one:

```c
vel_x = update_velocity_x ( pos_x, vel_x, loc_x, glb_x )
pos_x = update_position_x ( pos_x, vel_x )
vel_y = update_velocity_y ( pos_y, vel_y, loc_y, glb_y )
pos_y = update_position_y ( pos_y, vel_y )

curr = f ( pos_x, pos_y )

[ loc_val, loc_x, loc_y ] = ( curr < loc_val ) ? [curr, pos_x, pos_y] : [loc_val, loc_x, loc_y]
return curr
```

Given the dependencies highlighted by the pseudo-code, it's possible to benefit of a **pipeline introduction**.

`Figure Pipeline`

The vector nature of the problem, given by the position and the speed of an object in a multidimensional space, is prone to the use of **vectorization** to parallelize the computation in independent dimensions.
For the vectorization to be effective the operations to limit the boundaries of the solution space could be limited by the ALU's architecture, for example the modulo operator useful to provide a toroidal space couldn't be available, so this choice has to be experimental evaluated.

`Figure Pipeline+Vectorization`

Whilst the external loop can't be parallelized because of the dependency between consequent states, the internal one is fully exploitable because of the independent nature of the particles.
Once all the particles have been computed by the farm the collector analyzes the resulting objects to eventually update the global minimum.

`Figure Pipeline+Vectorization+Farm`

# Performance model

# Implementation details

# Experimental results
