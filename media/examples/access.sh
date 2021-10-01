clear
succeed "Found /lib/.hidden With SUID Bit By Unauthorized Root Access"
sleep 0.5
succeed "Found /proc/5384/  With SUID Bit By Elevated-Process Raw Byte Injection"
sleep 0.5
succeed "Found /var/.drpr   With Owner Authorized Root Access"
printf "\n"
sleep 0.5
notify  "Working On The Gather..."
sleep 0.5
failed  "An Error Occured While Sending Memory Dump Request...! Trying Again With SUID Enabled"
sleep 1
succeed "Memory Dump Returned Success Value 0, Saved in memdump.raw"
sleep 0.5
authenticate /tmp/cred
sleep 3
printf "\n"
read -p "Raw Data Byte Live Executor (Enter Command)::> "
warn "You Have Been Disconnected ..>!<.."
