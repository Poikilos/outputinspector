#!/usr/bin/env python
import sys


def warn(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def error(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def critical(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def fatal(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def pinfo(*args, **kwargs):
    '''
    Print info.
    '''
    print(*args, file=sys.stderr, **kwargs)


verbosity = 2

def set_verbosity(v):
    '''
    Set verbosity to 1 for verbose messages and 2 for debug messages.
    '''
    verbosity = v

def debug(*args, **kwargs):
    '''
    Only show the message if verbosity > 1 (See "set_verbosity").
    '''
    if verbosity > 1:
        print(*args, file=sys.stderr, **kwargs)
