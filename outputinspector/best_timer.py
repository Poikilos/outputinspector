import time

SPRING_AHEAD_WARNING = (
    'Warning: Your version of Python doesn\'t have'
    ' time.monotonic nor time.time, so using non-UTC'
    ' datetime seems to be the only option without pytz'
    ' (Therefore, running this program when the'
    ' DST time change occurs could cause'
    ' timed events to go too long, though timers will'
    ' only reset not fail on "Fall Back").'
)

try:
    best_timer = time.monotonic  # doesn't jump for DST
except AttributeError:
    # 'module' object has no attribute 'monotonic'
    try:
        '''
        try_timer = time.clock  # older versions of Python 2 or 3
        # Uh oh, it seems to be unreliable (uneven; tested on Linux;
        # it uses ctime directly which is CPU ticks).
        start = try_timer(); time.sleep(1.0); end = try_timer()
        tps = end - start
        if tps < 0.999:
            def best_timer():
                global tps
                return time.clock() / tps
                # ^ Still doesn't work (varies :( ). See also
                #   <https://stackoverflow.com/a/62702064/4541104>
        else:
            best_timer = try_timer
        '''
        def best_timer():
            fractional = time.time()
            remainder = fractional - float(int(fractional))
            # For some reason time.mktime rounds to the nearest second
            #   even though it is a float, so restore the remainder from
            #   the local time:
            return time.mktime(time.gmtime(time.time())) + remainder
            # ^ GMT should be ok, since GMT never has an offset (though
            #   some regions use BST (British Summer Time) during
            #   summer).

        # available in python 2.7.18
    except AttributeError:
        def best_timer():
            # sys.stderr.write(SPRING_AHEAD_WARNING+"\n")
            # sys.stderr.flush()
            # from datetime import timezone
            # For python version such as 2.7 where a datetime instance
            # (or class) doesn't have a timestamp() function.
            from datetime import datetime
            return (datetime.now() - datetime(1970, 1, 1)).total_seconds()
            # ^ Only delta (not datetime) has a total_seconds method, so
            #   get seconds since epoch.
            #   - TODO: UTC would avoid any jump for DST
            #     but there is code to compensate manually instead
            #     (See "timeout will be reset") due to:
            #     - timezone.utc doesn't exist in 2.7.18, so it
            #       can't be passed to datetime.now().
            #     - pytz doesn't exist in python 2 by default so
            #       pytz.timezone("UTC") can't be passed either.

