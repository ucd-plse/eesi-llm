"""Functions for common logging phrases."""

import glog as log

def log_finished(command, database_changed=True, partially_complete=False):
    """Logs that the command has finished and if database has updated."""

    if partially_complete and database_changed:
        log.info(f"{command} command has finished! Populated database with"
                 " only a PARTIAL amount of the expected entries."
                 " Check logs for more information.")
        return

    if database_changed:
        log.info(f"{command} command has finished! Populated database with"
                 " entries.")
        return

    log.info(f"{command} command finished! NO entries were populated in MongoDB."
             " Check logs for more information.")

def log_finished_file_output(command, file_written=False):
    """Logs that the command has finished and if an output file was written."""

    if not file_written:
        log.info(f"{command} command has finished! File was NOT written"
                 " successfully! Please check the logs of the related service"
                 " to find errors!")
        return

    log.info(f"{command} command has finished! File written successfuly!")
