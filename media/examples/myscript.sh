clear
hide cursor
cby "Loaded..!"
cbn "Test..!"
window 10 5 20 70
move 11 6
printf "Hello, This is Digle Scripting Example..!"
move 12 6
printf "Press Enter To Start "

read
clear
succeed "Request Hijacked Successfully.>!"
ask "Continue"

if  [ "$?" == 0 ];then
  exit
fi

succeed "Replaying ..."
sleep 1
succeed "Done, Request On Air"
sleep 1
succeed "Decoding Captured Bytes"
printf "\n"
inform "User    : Daniel Jack"
sleep 0.5
inform "Password: AS92fac00#aax3"
printf "\n"
sleep 2
succeed "Live Data Dump Finished"
sleep 5
unknown "Real Request From 192.168.134.146"
sleep 3
notify "Trying To Trigger BufferOverflow Request Result Buffer By Sending Malformed Result..."
sleep 1
succeed "Waiting For Connection"
sleep 0.2
succeed "Command Shell Opened"
sh
show cursor
