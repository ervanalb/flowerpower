#from RPi import GPIO
import time
#from contextlib import contextmanager

class FillError(Exception):
    pass

class FakePump:
    def __init__(self, fake_level=0, fake_capacity=10, fake_rate=1):
        self.fake_level = fake_level
        self.fake_capacity = fake_capacity
        self.fake_rate = fake_rate
        self.direction = 0
        self.last_time = None

    def _update(self, direction=None):
        now = time.time()
        if self.last_time is not None:
            self.fake_level += self.direction * (now - self.last_time) * self.fake_rate
            if self.fake_level > self.fake_capacity:
                print("OVERFULL", self.fake_level - self.fake_capacity)
            self.fake_level = max(0, min(self.fake_capacity, self.fake_level))
        if direction != self.direction:
            print("Level is", self.fake_level, "/", self.fake_capacity)
        self.last_time = now
        if direction is not None:
            self.direction = direction

    def flood(self):
        print("Flooding")
        self._update(1)

    def drain(self):
        print("Draining")
        self._update(-1)

    def off(self):
        print("Off")
        self._update(0)

    def full(self):
        self._update()
        return self.fake_level >= self.fake_capacity

class GPIOPump:
    def __init__(self, flood_pin, drain_pin, sense_pin):
        self.flood_pin = flood_pin
        self.drain_pin = drain_pin
        self.sense_pin = sense_pin
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(flood_pin, GPIO.OUT)
        GPIO.setup(drain_pin, GPIO.OUT)
        GPIO.setup(sense_pin, GPIO.IN, pull_up_down=GPIO.PUD_OFF)
        self.off()

    def flood(self):
        GPIO.output(self.flood_pin, GPIO.LOW)
        GPIO.output(self.drain_pin, GPIO.HIGH)

    def drain(self):
        GPIO.output(self.flood_pin, GPIO.HIGH)
        GPIO.output(self.drain_pin, GPIO.LOW)

    def off(self):
        GPIO.output(self.flood_pin, GPIO.HIGH)
        GPIO.output(self.drain_pin, GPIO.HIGH)

    def full(self):
        return not GPIO.input(self.sense_pin)

class Pot:
    def __init__(self, pump, flow_rate, max_container, max_extra_flood, max_extra_drain, fluid_estimate=None, container_estimate=None):
        self.pump = pump
        self.flow_rate = flow_rate
        self.max_container = max_container
        self.max_extra_flood = max_extra_flood
        self.max_extra_drain = max_extra_drain
        self.fluid_estimate = fluid_estimate
        self.fluid_estimate_good = fluid_estimate is not None
        self.container_estimate = container_estimate if container_estimate is not None else max_container
        self.container_estimate_good = container_estimate is not None

        self.fluid_setpoint = None
        self.error_message = None
        self.direction = 0
        self.last_direction = 0
        self.last_update = None
        self.error_on_setpoint_achieved = False

    def update(self):
        # Update state
        now = time.time()
        if self.last_update is not None:
            if self.last_direction != 0:
                self.fluid_estimate += self.last_direction * self.flow_rate * (now - self.last_update)
                if self.fluid_estimate > self.container_estimate:
                    self.container_estimate = self.fluid_estimate

        self.last_update = now
        self.last_direction = self.direction

        # Handle transitions
        if self.direction == 1 and self.fluid_estimate >= self.fluid_setpoint:
            if self.error_on_setpoint_achieved:
                # Filled more than expected. Uncalibrate the system
                self.fluid_estimate_good = False
                self.container_estimate = self.max_container
                self.container_estimate_good = False
                self.error("overfilled")
            else:
                self.off() # Done filling
        elif self.direction == -1 and self.fluid_estimate <= self.fluid_setpoint:
            if self.fluid_setpoint < 0:
                self.fluid_estimate_good = True
            self.off() # Done draining
        elif self.direction == 1 and self.pump.full():
            if self.fluid_estimate_good:
                self.container_estimate = self.fluid_estimate
                self.container_estimate_good = True
            self.off()

    def flood(self):
        self.error_message = None
        self.direction = 1
        self.pump.flood()
        self.update()

    def drain(self):
        self.error_message = None
        self.direction = -1
        self.pump.drain()
        self.update()

    def off(self):
        self.error_message = None
        self.direction = 0
        self.pump.off()
        self.update()

    def error(self, message):
        self.error_message = message
        self.direction = 0
        self.pump.off()
        self.update()

    def set_level(self, level_setpoint):
        level_setpoint = max(0, min(1, level_setpoint))

        if not (self.fluid_estimate_good or self.container_estimate_good) and level_setpoint not in (0, 1):
            self.error("uncalibrated")
            return

        if self.fluid_estimate is None:
            if level_setpoint == 0:
                self.fluid_estimate = self.max_container
            elif level_setpoint == 1:
                self.fluid_estimate = 0
        else:
            # Clamp the fluid estimate to within bounds
            # (it is set out-of-bounds by set_level(0) or set_level(1))
            self.fluid_estimate = max(0, min(self.container_estimate, self.fluid_estimate))

        self.error_on_setpoint_achieved = False
        if level_setpoint == 0:
            self.fluid_setpoint = -self.max_extra_drain
        elif level_setpoint == 1:
            self.fluid_setpoint = self.container_estimate + self.max_extra_flood
            self.error_on_setpoint_achieved = True
        else:
            self.fluid_setpoint = self.container_estimate * level_setpoint

        if self.fluid_setpoint > self.fluid_estimate and self.direction != 1:
            self.flood()
        elif self.fluid_setpoint < self.fluid_estimate and self.direction != -1:
            self.drain()

    @property
    def level(self):
        if not self.fluid_estimate_good or not self.container_estimate_good:
            return None
        return max(0, min(1, self.fluid_estimate / self.container_estimate))

    @property
    def fluid(self):
        if not self.fluid_estimate_good:
            return None
        return max(0, min(self.container_estimate, self.fluid_estimate))

    @property
    def container(self):
        if not self.container_estimate_good:
            return None
        return self.container_estimate

def test():
    try:
        pump = FakePump(fake_rate=5)
        pot = Pot(pump, flow_rate=5, max_container=15, max_extra_flood=5, max_extra_drain=3)
        pot.set_level(0)
        step = 0
        while True:
            print("D", pot.direction, "L", pot.level, "F", pot.fluid, "C", pot.container, "E", pot.error_message)
            pot.update()
            time.sleep(0.05)
            if pot.direction == 0:
                print("*** STEP {} COMPLETE ***".format(step))
                if step == 0:
                    pot.set_level(1)
                    step += 1
                elif step == 1:
                    pot.set_level(0.3)
                    step += 1
                elif step == 2:
                    pot.set_level(0.7)
                    step += 1
                elif step == 3:
                    pot.set_level(0)
                    step += 1
                elif step == 4:
                    pot.set_level(1)
                    step += 1
    finally:
        pump.off()

if __name__ == "__main__":
    #p1 = Pot(4, 27, 17, 0.88)
    #p2 = Pot(23, 22, 18, 1.02)
