# NEC driver

So it seems like this is the architecture that's the simplest:

- we have an edge detector ISR that records every edge into a queue
- we have a decoder state machine that normally transitions every time we take from the queue, however, if it takes too long to take from the queue, we transition back to the idle state. 
- There's also some additional counters that keep track of whether it's a repeat code, or whether there's a complete frame.
- after we get a repeat code or a complete frame, we just push that into another queue for consumers to decide what to do with that information.

an approximate NEC decoder state machine looks something like this:

![nec state machine decoder](./assets/nec_decoder.png)

The model runs everytime we detect an edge, but there's also an external timeout (through the queue) that essentially resets the state back to idle if it takes more than around 30ms because no pulse in the NEC protocol lasts more than around 10ms. This will also be where we can decide whether what we have is a repeat code.



