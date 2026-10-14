# GEDCOM Source Explorer

Cette distribution contient un binaire compilé pour le système et l'architecture indiqués dans le nom de l'archive, le fichier GEDCOM d'exemple et les ressources du rapport HTML.

Conservez l'arborescence du dossier et lancez les commandes depuis sa racine : le programme trouve le fixture et le template du rapport par chemins relatifs.

Afficher l'aide :

```bash
./gedcom-source-explorer --help
```

Analyser le GEDCOM d'exemple dans le terminal :

```bash
./gedcom-source-explorer gedcom-test.ged
```

Analyser un autre fichier :

```bash
./gedcom-source-explorer /chemin/vers/fichier.ged
```

Générer le rapport HTML par défaut :

```bash
./gedcom-source-explorer --html /chemin/vers/fichier.ged
```

Le rapport est écrit dans `build/report/index.html`; les feuilles CSS et le JavaScript nécessaires y sont déjà installés. Ouvrez ensuite ce fichier dans un navigateur. Aucun serveur ni Node.js n'est nécessaire à l'exécution du binaire.
