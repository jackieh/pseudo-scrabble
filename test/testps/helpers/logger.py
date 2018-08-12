# -*- coding: utf-8 -*-

"""Module for printing common types of message.
"""

from helpers import colors

class Logger:
    """Class for printing common types of messages.
    """

    def __init__(self, color_opt):
        """Set the color option in object state.
        """
        self.use_color = color_opt

    def println_warning(self, warning_message):
        """Print a warning prefixed by "Warning", possibly in yellow.
        """
        warning_begin = "Warning"
        if self.use_color:
            warning_begin = colors.str_bold_yellow(warning_begin)
        print(warning_begin + ": " + warning_message)

    def println_error(self, error_message):
        """Print an error prefixed by "Error", possibly in red.
        """
        error_begin = "Error"
        if self.use_color:
            error_begin = colors.str_bold_red(error_begin)
        print(error_begin + ": " + error_message)

    def warn_mysterious_file(self, file_name, file_location):
        """Warn the user about a mysterious file with which the test script
        doesn't understand what to do.
        """
        print_file_name = (colors.str_magenta(file_name)
                           if self.use_color else file_name)
        self.println_warning("Not sure what to do with file '"
                             + print_file_name + "' in " + file_location)

    def report_load_test(self, test_name, test_location):
        """Inform the user that a test has been successfully loaded.
        """
        load_test_begin = "Loading test"
        load_test_name = test_name
        if self.use_color:
            load_test_begin = colors.str_bold_cyan(load_test_begin)
            load_test_name = colors.str_magenta(load_test_name)
        print(load_test_begin + ": '" + load_test_name
              + "' in " + test_location)
