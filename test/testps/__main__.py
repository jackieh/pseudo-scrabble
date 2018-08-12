# -*- coding: utf-8 -*-

"""Execute tests by wrapping around the primary tester class.
"""

import os
import sys

import test_cli

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTS_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, os.pardir, "data"))
USE_COLOR = "--color" in sys.argv

def main():
    """Find tests in the tests directory and run
    tests if any have been successfully loaded.
    """
    tester = test_cli.CommandLineTester(TESTS_DIR, USE_COLOR)
    tester.get_tests()
    tester.run_tests()

main()
