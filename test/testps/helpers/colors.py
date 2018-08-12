# -*- coding: utf-8 -*-
# Make pylint not yell at me for wanting to use decorators to eliminate some
# boilerplate (unused-argument warning).
# pylint: disable=W0613

"""Internal module to apply color and bold formatting to strings.
"""

FORMAT_BOLD = "01;"
FORMAT_RED = "31"
FORMAT_GREEN = "32"
FORMAT_YELLOW = "33"
FORMAT_MAGENTA = "35"
FORMAT_CYAN = "36"

def format_str_color(function):
    """
    Args:
        function (str -> str): Function that returns a constant format template.

    Returns:
        str -> str: Function that applies color/bold formatting to a string.
    """
    start_format = "\33[%sm\33[K"
    end_format = "\33[m\33[K"
    def str_color(message):
        return (start_format % function(message)) + message + end_format
    return str_color

@format_str_color
def str_red(message):
    """Decorated function: Apply red color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_RED

@format_str_color
def str_bold_red(message):
    """Decorated function: Apply bold and red color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_BOLD + FORMAT_RED

@format_str_color
def str_yellow(message):
    """Decorated function: Apply yellow color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_YELLOW

@format_str_color
def str_bold_yellow(message):
    """Decorated function: Apply bold and yellow color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_BOLD + FORMAT_YELLOW

@format_str_color
def str_green(message):
    """Decorated function: Apply green color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_GREEN

@format_str_color
def str_bold_green(message):
    """Decorated function: Apply bold and green color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_BOLD + FORMAT_GREEN

@format_str_color
def str_magenta(message):
    """Decorated function: Apply magenta color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_MAGENTA

@format_str_color
def str_cyan(message):
    """Decorated function: Apply cyan color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_CYAN

@format_str_color
def str_bold_cyan(message):
    """Decorated function: Apply bold and cyan color format to a string.

    Args:
        str: String to be formatted.

    Returns:
        str: Formatted string.
    """
    return FORMAT_BOLD + FORMAT_CYAN
