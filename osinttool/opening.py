# Заходи в наш тгк за обновлениями! 
from colorama import Fore, Style, init
init()
# Цвета
dcyan = Style.NORMAL + Fore.CYAN
cyan = Style.BRIGHT + Fore.CYAN
blueb = Style.BRIGHT + Fore.BLUE
white = Style.BRIGHT + Fore.WHITE
baza = Style.NORMAL
# Баннер и меню
def Banner():
    print(f'''
{blueb}╔───────────────────────────────────────────────────────────────────────╗
{blueb}│                                                                       │
│   ████████╗░█████╗░░█████╗░██╗░░██╗░█████╗░███╗░░██╗
            ╚══██╔══╝██╔══██╗██╔══██╗╚██╗██╔╝██╔══██╗████╗░██║
            ░░░██║░░░██║░░██║██║░░██║░╚███╔╝░███████║██╔██╗██║
            ░░░██║░░░██║░░██║██║░░██║░██╔██╗░██╔══██║██║╚████║
            ░░░██║░░░╚█████╔╝╚█████╔╝██╔╝╚██╗██║░░██║██║░╚███║
            ░░░╚═╝░░░░╚════╝░░╚════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░╚══╝
╠───────────────────────────────────────────────────────────────────────╣
│                    {white}Переходник{dcyan}:{cyan} https://t.me/+R1DsCLCKjNE1YTNi                    {blueb}│
│              {white}Разработчики{dcyan}:{cyan} @hdideonon, @Kiloshi4kol              {blueb}│
│           {dcyan}[{white}!{dcyan}]{cyan}Пропишите /info для более подробной информации           {blueb}│
╚───────────────────────────────────────────────────────────────────────╝
{dcyan}[{white}ВНИМАНИЕ{dcyan}]{cyan} Софт является абсолютно новым, следить за новостями и обновлениями касательно софта можно в нашем телеграм-канале!''')
def Menu():
    print(f'''{blueb}╔─────────────────────╦─────────────────────────────────────────────────╗
│       {dcyan}[{white}OSINT{dcyan}]{blueb}       │           {dcyan}[{white}Разнообразные инструменты{dcyan}]{blueb}           │
╠─────────────────────╬─────────────────────────────────────────────────╣
├ {white}1{dcyan}. {white}Поиск по номеру{blueb}  ├ {white}5{dcyan}. {white}Добавить бд{blueb}                                  │
╠─────────────────────┼─────────────────────────────────────────────────╣
├ {white}2{dcyan}. {white}Поиск по IP{blueb}      ├ {white}6{dcyan}. {white}WormGPT{blueb}                                      │
╠─────────────────────┼─────────────────────────────────────────────────╣
├ {white}3{dcyan}. {white}Поиск по VK{blueb}      ├ {white}7{dcyan}. {white}Dos{blueb}                                          │
╠─────────────────────┼─────────────────────────────────────────────────╣
├ {white}4{dcyan}. {white}SHODAN{blueb}           ├ {white}8{dcyan}. {white}Web-crawler{blueb}                                  │
╚─────────────────────┴─────────────────────────────────────────────────╝''')
