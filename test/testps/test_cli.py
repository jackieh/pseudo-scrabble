# -*- coding: utf-8 -*-

"""Run tests on pseudo-scrabble CLI.
"""

import os

from helpers import logger

class CommandLineTester():
    """Run tests on the command line.
    """

    def __init__(self, tests_dir, color_opt):
        """Gather list of tests that exist in the expected directory.
        """
        self.dir = tests_dir
        self.log = logger.Logger(color_opt)
        self.tests = []

    def get_tests(self):
        """Add elements to the `tests` list.
        """
        input_names = []
        expected_names = []
        for full_file_name in os.listdir(self.dir):
            file_name, file_ext = os.path.splitext(full_file_name)
            if file_ext != ".txt":
                self.log.warn_mysterious_file(full_file_name, self.dir)
            else:
                test_tokens = file_name.split("-")
                test_name = "-".join(test_tokens[:-1])
                test_ext = test_tokens[-1]
                if test_ext == "input":
                    input_names.append(test_name)
                elif test_ext == "expected":
                    expected_names.append(test_name)
                else:
                    self.log.warn_mysterious_file(full_file_name, self.dir)
        input_names.sort()
        expected_names.sort()
        while input_names and expected_names:
            if input_names[0] < expected_names[0]:
                input_file = input_names[0] + "-input.txt"
                self.log.warn_mysterious_file(input_file, self.dir)
                del input_names[0]
            elif input_names[0] > expected_names[0]:
                expected_file = expected_names[0] + "-expected.txt"
                self.log.warn_mysterious_file(expected_file, self.dir)
                del expected_names[0]
            else:
                assert input_names[0] == expected_names[0]
                self.tests.append(input_names[0])
                self.log.report_load_test(input_names[0], self.dir)
                del input_names[0]
                del expected_names[0]
        for name in input_names:
            self.log.warn_mysterious_file(name + "-input.txt", self.dir)
        for name in expected_names:
            self.log.warn_mysterious_file(name + "-expected.txt", self.dir)

    def run_tests(self):
        """Run tests that were gathered in the constructor of this object.
        """
        self.log.println_error("'run_tests' function not yet implemented")
