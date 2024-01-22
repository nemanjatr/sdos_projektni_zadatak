# O Projektu
**** Projekat je u izradi ****
Ovaj projekat je sastavni dio kursa Sistemi za digitalnu obradu signala na Elektrotehničkom fakultetu u Banja Luci, akademske godine 2023/2024 u zimskom semestru. 
U sklopu projektnog zadatka potrebno je realizovati sistem za dodavanje muzičkih efekata u audio signal korištenjem razvojnog okruženja ADSP-21489. Pored realizacije sistema za obradu muzičkih efekata na razvojnoj ploči, potrebno je realizovati iste efekte i pomoću programskog jezika Python, a zatim uporediti rezultate dobijene na ploči sa rezultatima dobijenim u Python-u, te na osnovu toga napisati izvještaj.  

## Šta je neophodno za rad
Za rad sa razvojnom pločom ADSP-21489 potrebno je instalirati programski paket CrossCore Embedded Studio u kojem se, korištenjem prilagođenog programskog jezika C, može programirati ploča. Osim toga potrebno je instalirati programski jezik Python. U našem konkretnom slučaju koristićemo Anaconda distribuciju alata, koja u sebi sadrži Python interpreter, te u okviru nje Jupyter Notebook platformu za pisanje programa u Python programskom jeziku. 

## Osnovna ideja
Osnovna ideja projekta je da se na proizvoljnim uzorcima audio signala izvrši dodavanje određenih poznatih muzičkih audio efekata. Prije svega misli se na neke gitarske efekte, koji se mogu proizvoljno izabrati, ali u tekstu projektnog zadatka mogu se naći neki od predloženih najčešće korišćenih efekata.  
U konkretnom slučaju ovog projektnog zadatka izabrani si sljedeći efekti : kašnjenje (delay), vah-vah (wah-wah), fejzer (phaser), reverberacije (reverberation).
