# TODO

- [] write approximate methods to control $v$ and $\dot{\theta}$
- [] approximate that the motor's speed is approximately proportional to the motor speed
- [] figure out the motor's deadzone
- [] find optimal joystick deadzone
- [] use RTOS create tasks that handle polling, and writing commands to motors, and logging to the serial terminal
- [] turn it into a library
- [] write flowchart
- [] format code

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
&v_l = v - q \dot{\theta}
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

However, for joystick control, precision doesn't matter that much this is a good approximation.

Moreover, we won't directly control $v_l$ and $v_r$ because we don't have feedback control on the motors, rather, we will control the PWM signal. Since we're just doing joystick control, we can also make the approximation that our PWM signal $p$ is such, that $v \propto p$. We will make a slight modification to that to account for the motor's deadzone.
