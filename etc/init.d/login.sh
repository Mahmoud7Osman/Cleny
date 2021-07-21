source $PODGL/lib/colors2
source $PODGL/lib/fsh.cfg.sh

trap 'printf "\n"' SIGINT

if [ ! -f $PODGL/etc/passwd ]; then
   printf "${yellow} Create Your ${red}Digle${yellow} Password : "
   read pass
   printf "${yellow} Confirm Your Password: "
   read tmp
   if [ "$pass" != "$tmp" ];then
     cerr "Error: Password Confirmation Does not Match Entered Password.... Try Again.!"
     exit 1
   fi
   echo $pass > $PODGL/etc/passwd
fi

printf "Password: "
read epass
pass=$(cat $PODGL/etc/passwd)
if [ "$epass" != "$pass" ];then
   cerr "Incorrect Password !!"
   exec /bin/bash $PODGL/etc/init.d/login.sh
fi
printf "Starting ${red}Digle${white} Framework at$green `date`..\n"
printf "${red}Welcome$white $On_Blue$(whoami)\033[0m\n"
clog "Started The Digle Shell"

