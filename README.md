# PWL 3.5 — alimentation fantôme 48 V sur batterie, MSPM0C1104

Portage sur MSPM0 du firmware d'alimentation fantôme écrit pour MSP430.
Même fonction, autre architecture : Cortex-M0+ à la place du MSP430, en
VSSOP-20.

Version d'origine → [**CCS-MSP430-pwl**](https://github.com/Nicolboy/CCS-MSP430-pwl).

---

## Le portage

L'intérêt de ce dépôt est moins le firmware que le chemin pour y arriver. Le
journal, en tête du source, donne les étapes dans l'ordre où elles ont été
franchies :

| Date | Étape |
|---|---|
| 27/07/2024 | début de la migration G2452 → MSPM0C1104 |
| 28/07 | contrôle des GPIO |
| 29/07 | impulsions PWM |
| 31/07 | ADC |
| 02/08 | timer d'intervalle |
| 03/08 | conversion complète, mise au point |

Une semaine, périphérique par périphérique, chacun validé avant de passer au
suivant. C'est l'ordre qui compte : GPIO d'abord parce que sans lui on ne voit
rien, PWM ensuite parce que c'est la fonction, ADC après parce qu'il en dépend.

**`mymsp.h` et `mygpio_regs.h`** sont des définitions de registres écrites à la
main plutôt que reprises de DriverLib — utile pour comprendre ce que fait
réellement la couche d'abstraction TI, et pour s'en passer là où elle coûte
trop cher sur un composant de cette taille.

---

## Ce que fait le firmware

Identique à la version MSP430 : élévation vers 48 V par DC/DC polyphasé,
surveillance ADC multivoies, étalonnage en flash, protection de batterie à
plusieurs seuils puis veille.

---

## Compiler

Projet Code Composer Studio pour **MSPM0C1104**, sans RTOS, chaîne GCC.

| Fichier | Origine |
|---|---|
| `20250426-PWL3-5_prod.c` | le firmware |
| `mymsp.h`, `mygpio_regs.h` | définitions de registres, écrites à la main |
| `mspm0c1104.lds` | script d'édition de liens |
| `startup_mspm0c110x_gcc.c`, `ti_msp_dl_config.*` | **Texas Instruments** — démarrage et configuration DriverLib |

Les trois derniers gardent leurs en-têtes de copyright TI, condition de leur
licence BSD.

---

## Licence

[MIT](LICENSE), sauf les fichiers TI cités ci-dessus.
