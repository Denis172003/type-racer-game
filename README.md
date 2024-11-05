# TypeRacer Game

## Descrierea cerințelor
Proiectul TypeRacer simulează un joc în care utilizatorul trebuie să tasteze cuvinte afișate aleatoriu într-un interval de timp determinat. Acesta folosește un LED RGB pentru a semnaliza starea jocului și butoane pentru a controla rundele și nivelul de dificultate.

**Obiectiv:** Tastați cât mai multe cuvinte corecte într-o perioadă de 30 de secunde, ajustând dificultatea după preferințe.

- **LED RGB**:
  - Alb - starea de repaus
  - Verde - textul introdus corect
  - Roșu - textul introdus greșit
  - Clipire de 3 secunde - indicație de numărătoare inversă pentru începerea rundei.

- **Butonul Start/Stop**:
  - În modul de repaus - pornește o rundă nouă după o numărătoare inversă.
  - În timpul unei runde - oprește runda curentă.

- **Butonul de Dificultate**:
  - Permite utilizatorului să selecteze nivelul de dificultate (Easy, Medium, Hard).
  - Afișează mesajul “Easy/Medium/Hard mode on!” în terminal la fiecare schimbare.

## Componentele utilizate
- **Arduino UNO** (Microcontroller ATmega328P)
- **1x LED RGB** (indicator pentru starea jocului)
- **2x Butoane** (pentru start/stop runde și pentru selectarea dificultății)
- **5x Rezistoare** (3x 220/330 ohm, 2x 1000 ohm)
- **Breadboard**
- **Fire de legătură**

## Setup fizic

![image](https://github.com/user-attachments/assets/5691c48d-b394-40e2-9e89-b313d4bcdfa0)

## Descrierea codului

### 1. Starea de repaus
- LED-ul RGB este alb, iar jocul este inactiv, permițând utilizatorului să selecteze dificultatea sau să inițieze o rundă nouă.

### 2. Pornirea unei runde
- La apăsarea butonului de start în modul de repaus, LED-ul RGB clipește timp de 3 secunde (numărătoare inversă) pentru a indica începutul rundei.

```cpp
if (butonStart == LOW) {
    delay(20); // Debouncing
    if (!inRunda) {
        ledRGB_clipeste(); // LED-ul RGB clipește (numărătoare inversă)
        inRunda = true;
        startRunda(); // Funcție pentru a începe runda
    }
}
```
### 3. Setarea dificultății
Apăsarea butonului de dificultate în modul de repaus permite selectarea nivelului de dificultate, ciclând între Easy, Medium, și Hard. Mesajul corespunzător apare în terminal.

```cpp
if (butonDificultate == LOW && !inRunda) {
    delay(20); // Debouncing
    schimbaDificultate(); // Actualizează dificultatea
    Serial.println(dificultateSelectata); // “Easy/Medium/Hard mode on!”
}
```
### 4. Jocul propriu-zis
La începutul rundei, un cuvânt aleatoriu din dicționar este afișat în terminal. Utilizatorul introduce cuvintele și, dacă acestea sunt corecte, LED-ul RGB se aprinde în verde și un cuvânt nou este afișat imediat. Dacă este greșit, LED-ul devine roșu. Un timer bazat pe dificultate controlează intervalul de timp până la afișarea următorului cuvânt.

```cpp
if (inRunda) {
    cuvantCurent = dicționar[random(0, numarCuvinte)];
    afiseazaCuvant(cuvantCurent); // Afișează cuvântul curent
    if (verificaText(cuvantIntrodus)) {
        LED_RGB_verde();
        contorCorect++;
    } else {
        LED_RGB_rosu();
    }
}
```
### 5. Finalizarea rundei
După 30 de secunde, runda se termină, LED-ul RGB revine la alb, iar terminalul afișează scorul (numărul total de cuvinte scrise corect).

```cpp
if (timpScurs >= 30000) { // Runda de 30 secunde
    stopRunda(); // Oprește runda
    Serial.print("Scor final: ");
    Serial.println(contorCorect);
}
```
### 6. Resetarea jocului
La încheierea unei runde sau la apăsarea butonului de stop în timpul unei runde active, LED-ul revine la culoarea albă, iar jocul este pregătit pentru o nouă rundă.

```cpp
if (!inRunda) {
    LED_RGB_alb();
    Serial.println("Jocul este resetat.");
}
```
## Schema electrica
<img width="698" alt="Screenshot 2024-11-05 at 23 47 22" src="https://github.com/user-attachments/assets/e49645b1-6191-422b-a0ff-02752f94440a">


