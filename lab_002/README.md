# TODO

- [] make the queue sizes configurable in `config.h`
- [x] refactor logging across multiple
- [x] simplify the decision tree for the IR remote
- [x] write the flowchart
- [] rename some variables

The complete RTOS tasks necessary for this lab include:

- logging task
- mixer task
- NEC decoder task
- NEC RC task

How the tasks interact with one another is described here:

![](./assets/lab_002_task_diagram.svg)

The mixer task and the logging task is the same as lab 1, and further explanation of it is written here:

- [mixer task explanation](../docs/mixer.md)
- [logging task explanation](../docs/logging.md)

# NEC driver

So it seems like this is the architecture that's the simplest:

- we have an edge detector ISR that records every edge into a queue
- we have a decoder state machine that normally transitions every time we take from the queue, however, if it takes too long to take from the queue, we transition back to the idle state. 
- There's also some additional counters that keep track of whether it's a repeat code, or whether there's a complete frame.
- after we get a repeat code or a complete frame, we just push that into another queue for consumers to decide what to do with that information.

an approximate NEC decoder state machine looks something like this:

![nec state machine decoder](./assets/nec_decoder.png)

The model runs everytime we detect an edge, but there's also an external timeout (through the queue) that essentially resets the state back to idle if it takes more than around 30ms because no pulse in the NEC protocol lasts more than around 10ms. This will also be where we can decide whether what we have is a repeat code.

# NEC RC task

The important behavior that we have currently is that:

- when we press and hold the LEFT, RIGHT, UP, or DOWN button, the robot move accordingly
- when we release it, the robot stops

This is opposed to something like:

- when we press the LEFT, RIGHT, UP or DOWN button, make the robot move
- when we release it, do nothing
- when we press the STOP button, then command the robot to STOP

We choose the first behavior because I think it's a safer method to control the robot (because IR remote protocols aren't very long range, nor reliable), that is, whenever we loose the remote's repeat code signal, force stop the robot.

To do this, we rely on the fact that when we hold a button on a generic IR remote, a repeat code gets sent periodically (which I measured to be around 100ms), therefore, we can say that when we receive a repeat code, within some threshold, we do nothing, however, when we don't get the repeat code within that threshold time frame, then we shut the motors down automatically.

The NEC RC task will be the consumer of the NEC decoder task's output and decide what to do with it according to this flowchart:

![NEC RC flowchart](./assets/nec_rc_flowchart.svg)

# IR Remote command mapping

0: FF9867
1: FFA25D
2: FF629D
3: FFE21D
4: FF22DD
5: FF02FD
6: FFC23D
7: FFE01F
8: FFA857
9: FF906F
