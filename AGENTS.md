# Consignes pour les tests

- Chaque test unitaire doit suivre la structure AAA, avec des sections explicites `Given`, `When` et `Then`.
- Un test doit couvrir un seul scénario et un comportement précis. Les scénarios distincts, notamment les cas limites et les erreurs, doivent être des tests séparés.
- Un test peut contenir plusieurs assertions si elles vérifient les effets du même comportement; ne pas regrouper des comportements indépendants dans une seule fonction de test.
- Chaque fonction de test doit avoir un nom explicite et être précédée d'un commentaire qui décrit le comportement vérifié.
- Les helpers (préparation de contexte, capture des flux, fonctions de support) doivent être regroupés dans une section distincte, avant les fonctions de test. Ils ne doivent pas être enregistrés comme des tests.
- Les valeurs attendues doivent être déclarées comme constantes dans le bloc `Given`, puis réutilisées dans les assertions du bloc `Then`.
- Chaque test doit avoir ses propres données et son propre contexte afin de pouvoir être exécuté indépendamment des autres.
- Le runner doit nommer chaque test, rapporter son résultat séparément et poursuivre la suite après un échec.
