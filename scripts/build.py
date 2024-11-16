import sys
import argparse
sys.dont_write_bytecode = True # disable generation of __pycache__ folder

from cbuilder.pytils import logger

# builder modules
# @project: add supported platform here
from cbuilder import build_windows

import build_config

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Demorend build tools")
    parser.add_argument("--log-output",
            metavar = "LOGFILE",
            help = 'Output log file (default: build.log)',
            action = 'store', default = 'build.log')

    parser.add_argument('--log-level',
            metavar = 'LOGLEVEL',
            help = 'Log level for console',
            action = 'store', default = 'debug')

    subParsers = parser.add_subparsers(help = 'Possible actions', dest = 'action')

    # register builders
    # @project: add supported platform here
    build_windows.register(subParsers, build_config.config_windows)

    args = parser.parse_args()
    logger.initialize(args.log_output, args.log_level)
    logger.info("Demorend build tools")
    logger.debug(args)

    # execute builders
    try:
        args.execute(args)
    except Exception as e:
        logger.exception(e)
        sys.exit(1)
