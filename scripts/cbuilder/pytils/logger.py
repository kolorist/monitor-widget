import logging
import colorama

log = logging.getLogger(__name__)

class ColorizeFormatter(logging.Formatter):
    grey = colorama.Fore.WHITE
    green = colorama.Fore.GREEN
    yellow = colorama.Fore.YELLOW
    red = colorama.Fore.RED
    bold_red = colorama.Fore.RED
    reset = colorama.Fore.RESET
    format = "[%(asctime)s][%(levelname)-7s] %(message)s"

    FORMATS = {
            logging.DEBUG: grey + format + reset,
            logging.INFO: green + format + reset,
            logging.WARNING: yellow + format + reset,
            logging.ERROR: red + format + reset,
            logging.CRITICAL: bold_red + format + reset
            }

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)

def initialize(logFile, logLevelStr):
    colorama.init()
    log.setLevel(logging.DEBUG)

    if logFile != None:
        fileLogFormatter = logging.Formatter("[%(asctime)s][%(levelname)s][%(filename)s:%(lineno)s] %(message)s")
        fileHandler = logging.FileHandler(logFile, "w", "utf-8")
        fileHandler.setFormatter(fileLogFormatter)
        fileHandler.setLevel(logging.DEBUG)
        log.addHandler(fileHandler)

    consoleLogFormatter = ColorizeFormatter()
    consoleHandler = logging.StreamHandler()
    consoleHandler.setFormatter(consoleLogFormatter)

    logLevel = {
            'debug': logging.DEBUG,
            'info': logging.INFO,
            'warning': logging.WARNING,
            'error': logging.ERROR,
            }.get(logLevelStr, logging.DEBUG)

    consoleHandler.setLevel(logLevel)
    log.addHandler(consoleHandler)

def info(message):
    log.info(message)

def debug(message):
    log.debug(message)

def warning(message):
    log.warning(message)

def error(message):
    log.error(message)

def exception(message):
    log.exception(message)

def showProgressBar(percentage, hasErrors = False, customStr = ''):
    if hasErrors:
        print(colorama.Fore.RED, end = '')
    else:
        print(colorama.Fore.WHITE, end = '')

    print("\r[", end = '')
    scaledPercentage = int(percentage / 5)
    for i in range(0, 20):
        if i < scaledPercentage:
            print("=", end = '')
        elif i == scaledPercentage:
            if not hasErrors:
                print(">", end = '')
            else:
                print("x", end = '')
        else:
            print(" ", end = '')
    if percentage == 100 or hasErrors:
        print("] " + str(percentage) + "% " + customStr + colorama.Fore.RESET)
    else:
        print("] " + str(percentage) + "% " + customStr + colorama.Fore.RESET, end = '')
