# Consignes pour les tests

- Structurer chaque test selon AAA, avec des sections explicites `Given`, `When` et `Then`.
- Garder chaque test centré sur un scénario et un comportement observables. Séparer les comportements distincts, par exemple le cas nominal, les cas limites et les erreurs.
- Plusieurs assertions sont acceptables lorsqu'elles vérifient ensemble le même comportement; éviter de regrouper des vérifications sans lien dans un seul test.
- Donner à chaque test un nom qui décrit le comportement attendu. Ajouter un commentaire seulement s'il apporte une précision que le nom ne donne pas.
- Dans `Given`, déclarer les entrées, les résultats attendus et le contexte nécessaire au scénario. Rendre les valeurs attendues `const` lorsque c'est approprié et les réutiliser dans `Then`.
- Créer un contexte mutable neuf pour chaque test. Les fixtures immuables peuvent être partagées; ne pas faire dépendre un test de l'ordre d'exécution ou de l'état laissé par un autre.
- Garder les préparations propres à un scénario dans son `Given`. Factoriser une préparation commune dans un helper `setup_*` uniquement si cela réduit une duplication réelle ou clarifie le test. Appeler ce helper depuis le `Given` de chaque test concerné; il doit créer un état propre et ne pas exécuter le comportement testé.
- Organiser chaque fichier de test en sections nettement séparées : helpers, cas de test, puis runner (`main`). Garder les helpers avant les cas de test; réserver le préfixe `test_` aux fonctions qui représentent un cas de test. Le runner ne doit enregistrer que ces cas, jamais les helpers.
- Donner aux helpers un nom décrivant leur rôle (`setup_*`, `assert_*`, `capture_*`, etc.). Les garder dans le fichier de test tant qu'ils ne sont utiles qu'à ce fichier; les extraire dans un module partagé seulement lorsqu'ils servent plusieurs suites.
- Dans `When`, effectuer l'action principale testée, en limitant le bruit de préparation et les actions sans rapport.
- Dans `Then`, vérifier les résultats observables du comportement. Les échecs doivent fournir un diagnostic utile.
- Le runner doit identifier chaque test, rapporter son résultat séparément et continuer la suite après un échec.
