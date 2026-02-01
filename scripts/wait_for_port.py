import time
import serial.tools.list_ports

Import("env")


def wait_for_port(source, target, env):
    port = env.subst("$UPLOAD_PORT")
    timeout = 15
    interval = 0.5

    if not port:
        # No fixed port configured, let PIO auto-detect
        return

    print("Waiting up to %ds for %s..." % (timeout, port))
    start = time.time()
    while time.time() - start < timeout:
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if port in ports:
            print("Found %s" % port)
            return
        time.sleep(interval)

    print("Warning: %s not found after %ds, continuing anyway" % (port, timeout))


env.AddPostAction("upload", wait_for_port)
