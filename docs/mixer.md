
The mixer takes the variables that we command and outputs the variables that the robot can actually control.

For instance, in a quadcopter (drone), we may take in the desired roll, pitch and yaw in degrees per second and output the PWM commands for motor 1, motor 2, motor 3 and motor 4.

In our case, we take in 2 commands, desired linear velocity and desired angular velocity (approximately), and we output the PWM signal to the left motor and the right motor including its directions.

The mixing task simply takes in a normalized linear velocity and angular velocity command (ranges between 1 and -1), perform some calculations in order to get the desired normalized motor command to the left and the right motor. Also, the normalized linear and angular velocity is a shared variable between task, and must be protected using a mutex lock. This then gets mapped via a `motorWrite` function.

More about how we calculate the desired normalized motor command can be found in [this section](#mixing-matrix) and clipping will be dealt with [here](#mixing-and-dealing-with-clipping).

# Flowchart

A rough flowchart of the implementation of the mixer task

![mixer task flowchart](../lab_001/assets/flowcharts/lab_001_mixer_task.drawio(1).svg)


# Differential drive robot's kinematics

We want to control the vertical velocity $v$ and the angular velocity $\dot{\theta}$, but for the robot, we can only approximately control the robot's left wheel speed $v_l$ and right wheel speed $v_r$.

Suppose that the robot's center of mass is in the origin of our plane, and we're in the robot's reference frame.

For a differential drive robot, we essentially have a circle centered on the x-axis, with distance $R$ from the origin. Suppose that:

$$
R_l = R - q
$$

and

$$
R_r = R + q
$$

Where $q$ is the absolute distance between the center to each wheel.

Essentially, if we assume no slip, and the car drives in an arc of that circle, we can say that:

$$
v_l = R_l\dot{\theta}
$$

and:

$$
v_r = R_r\dot{\theta}
$$

$$
\begin{cases}
&v_l = (R - q) \dot{\theta}\\
&v_r = (R + q) \dot{\theta}
\end{cases}
$$

$$
\begin{cases}
&v_l = R \dot{\theta} - q \dot{\theta}\\
&v_r = R \dot{\theta} + q \dot{\theta}
\end{cases}
$$

Subtracting the two equations from one another:

$$
v_r - v_l = 2 q \dot{\theta}
$$

Therefore:

$$
\dot{\theta} = \frac{v_r - v_l}{2q}
$$

which is quite a nice result.

Now, let's say that we have vertical velocity $v$ in the car's center. Now, Newton's second law will eventually give you the fact that the velocity of the center of mass is the velocity in which a point mass representation of the rigid body will travel in.

Then:

$$
v = R \dot{\theta}
$$

So from the start:

$$
\begin{cases}
&v_l = v - q \dot{\theta}\\
&v_r = v + q \dot{\theta}
\end{cases}
$$

So:

$$
v_l + v_r = 2v
$$

Meaning:

$$
v = \frac{v_l + v_r}{2}
$$

If we were to define:

$$
\begin{cases}
&q\dot{\theta} = \Delta v\\
\end{cases}
$$

$$
v_r = v + \Delta v
$$

$$
v_l = v - \Delta v
$$

Meaning:

$$
\dot{\theta} = \frac{\Delta v}{q}
$$

So what does this mean?

This means that controlling $v$ controls the speed of the car's center in the forward direction, and then controlling the parameter $\Delta v$ which directly commands the car's left and right wheel allows us to control our angular velocity $\dot{\theta}$.

# Approximation

So, for a 4 wheeled drive differential steering robot, the differential drive model isn't perfect, because all 4 wheels do not satisfy the no slip condition for turning around an arc of a circle.

However, for joystick control, precision doesn't matter that much since this is a good approximation.

Moreover, we won't directly control $v_l$ and $v_r$ because we don't have feedback control on the motors, rather, we will control the PWM signal. Since we're just doing joystick control, we can also make the approximation that our PWM signal $p$ is such, that $v \propto p$. We will make a slight modification to that to account for the motor's deadzone.



# Mixing matrix

Now, for a normal differential drive robot, we have 2 input which are linear velocity command (or throttle) $u_t$ and angular velocity command $u_a$ , and we have 2 output, which is $u_l$ for left motor command and $u_r$for right motor command.

We then have:

$$
\begin{cases}
&u_l = u_t - u_a\\
&u_r = u_t + u_a\\
\end{cases}
$$

Essentially:

$$
\begin{bmatrix}
u_l\\
u_r
\end{bmatrix} =
\begin{bmatrix}
1&-1\\
1&1\\
\end{bmatrix}
\begin{bmatrix}
u_t\\
u_a
\end{bmatrix}
$$

Our sign convention now is such that positive $u_l$ makes the robot moves forward.

However, for this particular robot, we can command each of the 4 motors separately, So in fact, we must do:

$$
\begin{bmatrix}
u_{m0}\\
u_{m1}\\
u_{m2}\\
u_{m3}
\end{bmatrix} =
\mathbf{S}
\begin{bmatrix}
u_l\\
u_r
\end{bmatrix}
$$

So, should I already multiple the sign matrix $\mathbf{S}$ and the mixing matrix $\mathbf{M}$ or should I use the decomposition? Essentially, I think this is the case because it might be possible to use mechanum wheels on the robot, so I want to make the convention similar to how we can use the same cleanflight software to control not only quad x architecture but hexcopters too. It's probably more configurable if I were to use the decomposition.

The sign matrix not only selects which motor is left or right, it also selects the sign, and in this case, it will follow the hardware's sign where positive is going to be:

- IN1 HIGH
- IN2 LOW

etc.

# Mixing and dealing with clipping

Suppose that after rates, joystick deadzone, etc, we eventually get the following:

```c
typedef struct {
  float linear;
  float angular;
} Speed;
```

Where linear is going to be in between -1 and 1, and similarly for angular velocity.

So, because this is a differential drive robot, it's best to model it using 2 output, left and right motor. There's also an additional sign remapping afterwards because we actually have 4 motors.


So what we currently have, assuming that everything is normalized:

$$
\begin{cases}
&u_l = u_t - \theta\\
&u_r = u_t + \theta\\
\end{cases}
$$

Where $u_t$ is between -1 and 1, and $\theta$ is in between -1 and 1 at its max.

So what can we do to preserve shape? I scale everything down essentially, so here's what we're going to do, we'll define:

- if either $|u_l|$ or $|u_r|$ is bigger than 1, take the biggest one and store it as $u_m = max(|u_l|, |u_r|)$
- then what we do is both: $\frac{u_l}{u_m}$ and $\frac{u_r}{u_m}$

