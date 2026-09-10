# NEC driver

So it seems like this is the architecture that's the least scuff:

- we have an ISR that will just store all of the edges in a ring buffer which can be revealed to the user as a sort of queue, but it push and pop should be atomic.

Now, if we had that data, we now need to implement some sort of state machine to do its best to decode it.

# References

- [amebaiotdocuments](https://amebaiotdocuments.readthedocs.io/en/latest/ambd_arduino/AMB21/examples_and_components/Peripheral%20Examples/IR%20-%20Transmit%20IR%20NEC%20Raw%20Data%20And%20Decode.html)
- [hackster](https://www.hackster.io/news/this-tinydecoder-is-an-ir-remote-receiver-and-nec-decoder-powered-by-an-attiny13a-96b81f153397)
- [hackaday](https://hackaday.io/project/169566-tiny-pcb-weighing-25-grams-w-a-few-features/log/177625-nec-decoding-in-c)
- [simple circuit](https://simple-circuit.com/nec-remote-control-decoder-pic16f887-mikroc/)

- [sbprojects](https://www.sbprojects.net/knowledge/ir/nec.php)
- [daily.dev](https://daily.dev/posts/nec-protocol-driver-infrared-remote-embedded-system-project-series-20-coen4vdxt)


