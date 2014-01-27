# Mosh Cleaner

Mosh cleaner kills stale mosh sessions, according to this algorithm:

    For each user:
    	If user only has one mosh session:
    		continue
    	For each mosh session other than the most recent of that user:
    		If session is currently connected:
    			continue
    		If session is older than kill-time:
    			kill session

`kill-time` begins at 24 hours and splits in half each time, until a minimum of 1 hour. This amounts to the following semantics:

* Most recent session: never killed
* Any currently connected session: never killed
* Second most recent session: killed if not used for 24 hours
* Third most recent session: killed if not used for 12 hours
* Forth most recent session: killed if not used for 6 hours
* Fifth most recent session: killed if not used for 3 hours
* Sixth most recent session: killed if not used for 1.5 hours
* Seventh most recent session: killed if not used for 1 hour
* Eighth most recent session: killed if not used for 1 hour
* Ninth most recent session: killed if not used for 1 hour

### Usage

    $ make
    $ sudo make install
    $ sudo clean-mosh
