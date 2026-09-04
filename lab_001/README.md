# TODO

- [x] write approximate methods to control $v$ and $\dot{\theta}$
- [] measure button bounce back and measure it
- [] investigate why left and right button aren't responsive for interrupts
- [] approximate that the motor's speed is approximately proportional to the motor speed
- [] figure out the motor's deadzone through experimental means
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
