# TODO

- [x] write approximate methods to control $v$ and $\dot{\theta}$
~~- [] measure button bounce back and measure it~~
~~- [] investigate why left and right button aren't responsive for interrupts~~
~~- [] approximate that the motor's speed is approximately proportional to the motor speed~~
- [x] figure out the motor's deadzone through experimental means
- [x] find optimal joystick deadzone
- [x] use RTOS create tasks that handle polling, and writing commands to motors, and logging to the serial terminal
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

# Joystick

The joystick ADC will range from `adc_min` to `adc_mid - (adc_max - adc_min) * deadzone` which maps to $[-1, 0]$.

And then we do something similar for $[0, 1]$

From there, we can just linearly map this to an internal variable normalized variable called "speed" or something, that is normalized such that 0 means stop, 1 means full speed, and anywhere in between is approximately linearly interpolating between the speeds. We can also make the end point adjustable.

We want one for linear speed, and one for angular velocity.

# Motors

We will do a similar mapping from speed going from `[0, 1]` to `[deadzone * (1 << resolution),1 << resolution]` and similarly for the reversed direction.

# Debounce logic

Essentially, we have a task that does debouncing, and we use this notification thing built into RTOS. 

```c
void buttonTask(void *pvParameters)
{
    for (;;)
    {
        // Sleep until someone notifies us
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // We were notified — do the work once
        doSomething();

        // Loop back and sleep again
    }
}
```

To wake it:

```c
xTaskNotifyGive(buttonTaskHandle);
```

To wake it from an ISR:

```c
void IRAM_ATTR buttonISR()
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(
        buttonTaskHandle,
        &higherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
```

Now, to send data, such as sending the button pin:

```c
xTaskNotifyFromISR(
    task,
    BUTTON_UP,
    eSetValueWithOverwrite,
    &woken
);
```

To pass data to an ISR:

```c
void IRAM_ATTR buttonISR(void *arg)
{
    uint32_t button = (uint32_t)arg;

    // notify task with `button`
}
```

and:

```c
attachInterruptArg(UP_PIN,    buttonISR, (void *)BUTTON_UP,    RISING);
attachInterruptArg(DOWN_PIN,  buttonISR, (void *)BUTTON_DOWN,  RISING);
attachInterruptArg(LEFT_PIN,  buttonISR, (void *)BUTTON_LEFT,  RISING);
attachInterruptArg(RIGHT_PIN, buttonISR, (void *)BUTTON_RIGHT, RISING);
```

It seems like we can't really press then release faster than 150 ms. As for debouncing:

```
12, 14247
14, 14250
15, 14253
```

Where the right side is in milliseconds.

So, it seems like around 20 ms delay is enough for debouncing.

## RTOS queue creation

```
QueueHandle_t logQueue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogMessage));
```

Taking it out:

```
xQueueReceive(logQueue, &msg, portMAX_DELAY);
```

Pushing it to the queue:

```
xQueueSend(logQueue, &msg, 0);
```

# Mixing and dealing with clipping

Suppose that after rates, joystick deadzone, etc, we eventually get the following:

```
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

# Deadzone mapping and remapping

It's probably best to implement this as a pure function. call it `asymMap` and call it `invAsymMap`, where:

- `asymMap` requires a `minLeft, maxLeft and minRight, maxRight, raw` and spits out a scalar called `scaled` between `-1` and `1` (this will be used for joystick ADC to actual motor commands)
- `invAsymMap` requires a `minLeft, maxLeft, minRight, maxRight, scaled` and spits out something called `raw` between `minLeft` and `maxRight` (this will be used for commanding motor's PWM).

There's however, some trouble that must be solved when we need to invert it. Essentially, for `asymMap`, normally, it would be like:

- `[minLeft, maxLeft]` -> `[-1, 0]`
- and `[maxLeft, minRight]` -> 0
- and `[minRight, maxRight` -> `[0, 1]`

When we invert it however:

- `[minRight, maxRight] -> [0, -1]`
- middle is same
- `[minLeft, maxLeft] -> [1, 0]`

So I guess the easiest way is to flip the sign of the mapped output?

Now, I guess we could say it's the same for `invAsymMap`.

# Sign convention

For normal hobby RC, the sign convention is as such:

```
y+
^
|
---> x+
```

So, it's kinda interesting, because, if look at the x-y plane of the drone, normal mapping without sign change would mean that (let's say that this is the left stick) positive $\dot{\theta}$ is CW, which isn't the common mathematical convention. I think it's best to use the normal mathematical sign convention, meaning that we must flip it.

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

# Information propagation

So essentially, `mixer.h` should be its own isolated file that doesn't depend on `rc.h` because we should be able to use it in other projects too, like, for remote controlled robot or bluetooth controlled robot.

As such, the internal struct `speedLimit` and `speed` should belong to `mixer.h`.

Now, there's a problem of race conditions. As such, we should have mutexes that protect both `speedLimit` and `speed`.

There are also two architectures for notifying to update the motors.

- We can have a separate motor task that independently reads `speed` and updates it accordingly
- We can have a motor task that waits until a task notifies that it has updated the speed variable, and then have the motor task run, of course, with mutex for protection just in case.

I know that in cleanflight, we have a separate IMU task, and a separate PID task that can run at different frequencies, and I guess the PID task's frequency would then need to be synchronized to the mixer task, and also motor command task, which means notification is a better architecture?

The cool thing about the notification architecture is that we can easily turn it into a separate asynchronous task which operates at its own frequency by simply having a task that notifies that task every fixed period.

Also, this means that our tasks should be started in the following order:

- logging task
- mixer task
- rc task

For it to behave nicely while everything is starting. It's no wonder why `systemd` is so complicated because of this problem of "start up dependency".




