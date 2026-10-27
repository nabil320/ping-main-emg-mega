# Main prothétique — EMG et Arduino Mega

Dossier technique illustré : acquisition EMG, connexion à la Mega, calibration, décision et préparation de la commande de moteurs à courant continu.

## Consulter

Ouvrir `index.html` dans un navigateur ou consulter le site GitHub Pages une fois activé. Les images, schémas, interactions et le programme téléchargeable sont intégrés au HTML : aucun service externe n’est requis pour lire le dossier.

## Publier sur GitHub Pages

Dans les paramètres du dépôt : **Pages → Deploy from a branch → main → / (root)**, puis enregistrer. GitHub affiche ensuite l’adresse de publication.

## Mettre à jour

Remplacer `index.html` par la nouvelle version et créer un commit sur `main`. Avec la publication depuis cette branche, GitHub Pages republie le site à la même adresse ; les lecteurs doivent actualiser la page. Une modification locale non envoyée à GitHub ne modifie pas le site.

## Programme Arduino

`arduino/mega_emg.ino` réalise l’acquisition et la décision uniquement. Il ne pilote pas les moteurs. Le code reste à compiler et valider sur votre matériel.

## Sources et illustrations

Les sources techniques et l’attribution des figures sont incluses dans le dossier. Les figures des articles et les captures marchandes conservent les droits de leurs auteurs respectifs. Aucun choix de moteur ou de batterie n’est présenté comme définitivement validé.
