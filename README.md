# O Projektu
Ovaj projekat je sastavni dio kursa Sistemi za digitalnu obradu signala na Elektrotehničkom fakultetu u Banja Luci, akademske godine 2023/2024 u zimskom semestru. 
U sklopu projektnog zadatka potrebno je realizovati sistem za dodavanje muzičkih efekata u audio signal korištenjem razvojnog okruženja ADSP-21489. Pored realizacije sistema za obradu muzičkih efekata na razvojnoj ploči, potrebno je realizovati iste efekte i pomoću programskog jezika Python, a zatim uporediti rezultate dobijene na ploči sa rezultatima dobijenim u Python-u, te na osnovu toga napisati izvještaj.  

## Šta je neophodno za rad
Za rad sa razvojnom pločom ADSP-21489 potrebno je instalirati programski paket CrossCore Embedded Studio u kojem se, korištenjem prilagođenog programskog jezika C, može programirati ploča. Osim toga potrebno je instalirati programski jezik Python. U našem konkretnom slučaju koristićemo Anaconda distribuciju alata, koja u sebi sadrži Python interpreter, te u okviru nje Jupyter Notebook platformu za pisanje programa u Python programskom jeziku. 

## Pokretanje programa
Za pokretanje potrebni su gore navedeni softverski paketi. Dakle CCES za C implementaciju, kao i ploča ADSP-21489 (može se koristiti i simulator u CCES, ali on ne daje uvijek vjerodostojne rezultate).  
A za python implementaciju jednostavno se preuzme i pokrene notebok pod nazivom gitarski_audio_efekti.ipynb (ako su instalirane gore navedene distribucije i paketi, može se podesiti i koristit i Visual Studio Code). 

## Osnovna ideja
Osnovna ideja projekta je da se na proizvoljnim uzorcima audio signala izvrši dodavanje određenih poznatih muzičkih audio efekata. Prije svega misli se na neke gitarske efekte, koji se mogu proizvoljno izabrati, ali u tekstu projektnog zadatka mogu se naći neki od predloženih i najčešće korišćenih efekata.  
U konkretnom slučaju ovog projektnog zadatka izabrani si sljedeći efekti : kašnjenje (delay), distorzija (distortion), vah-vah (wah-wah), fejzer (phaser), reverberacije (reverberation).
Nakon realizacije efekata, treba iskoristiti optimizacione tehnike različite vrste da se pokuša doći do unapređenja u pogledu brzine izvršavanja koda ili memorijskog zauzeća. Sve vrijeme treba mjeriti broj utrošenih ciklusa na pojedine algoritme, te rezultate propisno dokumentovati. 
Na kraju treba uporediti dobijene rezultate (konkretne odmjerke) dobijene u CCES i Python-u i komentarisati evenutalna odstupanja i slično.

## Sadržaj repozitorijuma
Repozitorijum je organizovan tako da postoje dva glavna foldera. Prvi je kod za pokretanje na ploči u folder adsp_implementacija, gdje je u src folderu glavni kod programa, uz pomoćne .h fajlove. Fajl test_audio.h sadrži odmjerke ulaznog signala za obradu. Fajlovi iir_peak_a(b).h sadrži odmjerke koeficijenata IIR filtra korištenog u efektu wah_wah, slično je i za iir_notch_a(b).h koji se koriste u phaser efektu.  
Drugi folder je python_implementacija i on sadrži jupyter notebook skriptu, kao kombinaciju izvještaja i koda. Tu se nalazi realizacija svih efekata u python programskom jeziku, te komentari realizacije, kao i teorijska pozadina efekata. Također tu je generisan i testni signal koji je korišten u CCES realizaciji. Na kraju tog notebook-a nalazi se testiranje rezultata. Učitavani su fajlovi iz foldera rezultati, gdje se nalaze rezultati obrade iz dvije implementacije. Iscratni su i grafici razlika tih rezultatnih odmjeraka.  
U korijenskom direktorijumu nalazi se izvještaj u pdf formi.
