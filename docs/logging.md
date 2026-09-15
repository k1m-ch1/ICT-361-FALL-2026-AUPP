
Logging to the Serial monitor is handled by a logging task, mainly for the following reason:

- when a task needs to log something, we don't want it to spend time calling `Serial.print` which takes a while (most important)
- we want logging to be well formatted
- we don't want 2 tasks logging to the same `Serial` at the same time (rare, but might happen)

The logging task is extremely simple. The logging task has one queue (non-blocking) and takes from the queue in a blocking way. When a task writes to the queue, we just take the struct, unpack it, and print it to through the UART in a standardized format, which logs the timestamp in milliseconds since the microcontroller has started, and roughly where the source came from.

This is a rough flowchart of the logging task:

![](../lab_001/assets/flowcharts/lab_001_logging_queue.drawio.svg)
