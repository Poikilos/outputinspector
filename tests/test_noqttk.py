from __future__ import print_function
import sched
import sys
import unittest
import time
import os
import threading

TESTS_DIR = os.path.dirname(os.path.realpath(__file__))
REPO_DIR = os.path.dirname(TESTS_DIR)

if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)

from outputinspector.best_timer import best_timer as second_timer
from outputinspector.noqttk import (
    noqt_tick,
)


class TestLogging(unittest.TestCase):
    '''
    The print_something event is used for parallel processing but
    Python 2 only has parallel processing with the threading module
    since it doesn't have the asyncio module (and await etc.).
    Scheduler is only able to run parallel if polled, correct me if
    I'm wrong (and correct the test_best_timer method). See
    test_threading_timer for the algorithm (*much* simpler) chosen for
    noqt.QTimer.
    '''
    def test_best_timer(self):
        print("")
        print("Testing sched...")
        self._interval = 1
        self._scheduler = sched.scheduler(second_timer, time.sleep)
        # ^ default scheduler values (optional as of 3.3):
        #   timefunc=time.monotonic, delayfunc=time.sleep
        # ^ There are no default arguments in earlier versions of Python
        #   (also note QTimer uses ms and scheduler uses s)
        self._scheduler.enter(self._interval, 1, self.print_something)
        print("* printing something in {}...".format(self._interval))
        self._event = self._scheduler.run(blocking=False)
        # ^ default blocking=True
        wait_sec = 2
        for i in range(wait_sec):
            time.sleep(1)
            result = self._scheduler.run(blocking=False)
            # returns time until completion if not complete
        print("* done waiting {} second(s) for the event.".format(wait_sec))
        '''
        TODO: stopping a scheduler requires some odd steps
          (See <https://stackoverflow.com/a/12907698/4541104>):
            if self._event:
                self._scheduler.cancel(self._event)
                self._event = None

        '''

    def test_threading_timer(self):
        self.last_message = ""
        print("")
        print("Testing threading.Timer...")
        self._interval = 1
        self._timer = threading.Timer(self._interval, self.print_something)
        self._timer.start()
        print("* doing other things at the same time")
        time.sleep(self._interval + .25)
        print("* trying to cancel a timer...")
        self._timer2 = threading.Timer(self._interval, lambda me=self: me.print_something_else())
        self._timer2.start()
        time.sleep(self._interval / 2.0)
        self._timer2.cancel()
        time.sleep(self._interval / 2.0 + .1)
        self.assertEqual(self.last_message, "something")
        # ^ should not be "something else", since stop was called
        print("* Ensuring extra cancel calls don't raise exceptions...")
        self._timer2.cancel()
        self._timer2.cancel()

    def print_something(self):
        print("* something")
        self.last_message = "something"

    def print_something_else(self):
        print("* something else")
        self.last_message = "something else"


if __name__ == "__main__":
    testcase = TestLogging()
    count = 0
    for name in dir(testcase):
        if name.startswith("test"):
            count += 1
            fn = getattr(testcase, name)
            fn()  # Look at def test_* for the code if tracebacks start here
    if count > 0:
        print("All {} test(s) passed".format(count))

