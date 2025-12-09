PROJET IOT : MISE À JOUR OTA SÉCURISÉE & GESTION D'ÉNERGIE
Auteurs : Mehdi DERBAL, Chiheb HBIBI, Moetez CHIHI

--- CONTENU DU DOSSIER ---
1. /Server : Contient le script Python (Flask) et la Base de données.
2. /Firmwares_Source : Les codes sources Arduino (V1, V2, V3).
3. /Documentation : Rapport et Présentation.

--- COMMENT LANCER LE PROJET ---
1. Ouvrir un terminal dans le dossier /Server.
2. Lancer le serveur : python secure_server.py
3. Le serveur écoute sur https://0.0.0.0:8000
4. Pour lire la base de données : python liredb.py

--- NOTES ---
- Le certificat SSL est auto-signé (fichiers .pem fournis).
- L'IP du serveur est configurée en dur dans le code Arduino (10.69.0.179), à modifier si besoin.
