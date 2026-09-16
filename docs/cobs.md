COBS encoding is pretty simple solution to the packet framing problem.

The issue is that COBS only sorts of guarantee framing assuming that the transmission is reliable. We can obviously do some sort of rudimentary checking like, if the final pointer doesn't point to a 0x00, disregard the whole frame, but honestly, even without guarantees, something like:

- if pointer doesn't match up (especially for the last byte), destroy the whole thing

BTW, should I dynamically allocate at the decoding stage? I wish we could but if we want to make the decoder naive, that is, its job is only to take a frame in transmission and turn it into an actual frame, then we shouldn't dynamically allocate it... Should i just have like a fixed size buffer? I guess that's the simplest solution, we're not doing anything super sophisticated. I mean, the reason why we're doing this in the first place is because ASCII based protocol decoding is a bit scuff and hard reason and make it fool-proof.

- having a packet id as the first byte to encode a struct

# Implemenation

A simple decoder can be something like:

- chime in, read byte and ignore everything until we see a 0x00
- now, parse the next byte as the pointer to the next 0x00
- take that byte and do a loop that read byte by byte, or perhaps use some sort of readNBytes if the api has it
- push that into my soon to be complete struct
- add a 0x00 and push that to that struct too
- now go back to the read parse next byte as pointer to the next 0x00 stage
- ...

When encoding, add a `0x00` when we're first starting because we want to ensure the listener is synchronized (or something else because `0x00` is super common. It's ideal to use something like `0xAA` or something that doesn't really appear in the ASCII's 0 to 9, A-z, but honestly, it doesn't matter that much). I think `0xAA` is a good choice.
