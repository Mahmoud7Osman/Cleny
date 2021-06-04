source $PODGL/lib/colors

# Output
cout(){
printf $green"☢$white %s\n" "$*"
}
# Error
cerr(){
printf $red"☢$white %s\n" "$*"
}
# logger
clog(){
printf "$green`whoami`$white @ `date` $yellow%s\n" "$*" >> $PODGL/var/log/slog.bgl
}
# Check Box Yes
cby(){
printf $white"["$green" ✔ "$white"]$white %s\n" "$*"
}
# Check Box No
cbn(){
printf $white"["$red" ✘ "$white"]$white %s\n" "$*"
}

cin(){
printf "$white┌──[$cyan$PWD$white]----[$green$VICTIM$white]-[$purple$PORT$white]\n└─"
read -p "$(printf $PS1) " -e $1
printf "$green"
}
olog(){
if [ -f "var/log/slog.bgl" ];then
 cout "Opening Log Data Holder... (src = var/log/slog.bgl)"
 cat $PODGL/var/log/slog.bgl
else cerr "Logs Holder Dosn't Exists"
fi
}
rlog(){
if [ -f "var/log/slog.bgl" ];then
 cout "Removing var/log/* ... !"
 sleep 0.3
 rm $PODGL/var/log/* $PODGL/var/digle_history
else cerr "No Logs Found"
fi
}

victim(){
  case $1 in
  ip) export VICTIM=$2;;
  port)export PORT=$2;;
  *) cerr Invalid Usage;;
  esac
}
