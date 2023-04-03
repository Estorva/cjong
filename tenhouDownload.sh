# Usage: tenhouDownload <URL>
# Input URL example: https://tenhou.net/0/?log=2023020601gm-0009-0000-33fbd68e&tw=1
# Input URL example: https://tenhou.net/0/?log=2023020601gm-0009-0000-33fbd68e
# CURL from URL: https://tenhou.net/0/log/?2023020601gm-0009-0000-33fbd68e

# remember to escape `&' in terminal

# trim off &
LOGID=${1%\&*}
LOGID=${LOGID#*=}
curl "https://tenhou.net/0/log/?${LOGID}" -o gamelog/$LOGID.mjlog
