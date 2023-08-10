#!/usr/bin/env python3
import sys
import re

from outputinspector.mainwindow import main


if __name__ == "__main__":
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    sys.exit(main())
