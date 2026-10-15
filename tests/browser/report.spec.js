const { test, expect } = require("@playwright/test");

test.describe("rapport GEDCOM", () => {
  let pageErrors;

  test.beforeEach(async ({ page }) => {
    pageErrors = [];
    page.on("pageerror", (error) => pageErrors.push(error.message));
    await page.goto("/");
  });

  test.afterEach(() => {
    expect(pageErrors).toEqual([]);
  });

  test("navigue entre les sources", async ({ page }) => {
    // Given
    await expect(page.locator("#page-position")).toHaveText("1 / 4");
    await expect(page.locator(".source-title")).toHaveText("@S1@");

    // When
    await page.getByRole("button", { name: "Source suivante" }).click();

    // Then
    await expect(page.locator("#page-position")).toHaveText("2 / 4");
    await expect(page.locator(".source-title")).toHaveText("@S2@");

    // When
    await page.getByRole("button", { name: "Dernière source" }).click();

    // Then
    await expect(page.locator("#page-position")).toHaveText("4 / 4");
    await expect(page.locator(".source-title")).toHaveText("@S4@");
  });

  test("filtre les sources sans tenir compte des accents", async ({ page }) => {
    // Given
    await expect(page.locator("#source-count")).toHaveText("4 / 4");

    // When
    await page.locator("#source-filter").fill("cagouille futee");

    // Then
    await expect(page.locator("#source-count")).toHaveText("1 / 4");
    await expect(page.locator("#source-navigation .source-nav-item"))
      .toHaveCount(1);
    await expect(page.locator(".source-title")).toHaveText("@S2@");

    // When
    await page.locator("#source-filter").fill("");

    // Then
    await expect(page.locator("#source-count")).toHaveText("4 / 4");
  });

  test("affiche les dépôts et citations associés à une source", async ({ page }) => {
    // Given
    await expect(page.locator(".source-title")).toHaveText("@S1@");

    // Then
    await expect(page.locator("#repository-section"))
      .toContainText("@R1@ · Archives Municipales");
    await expect(page.locator("#citation-section .related-entry"))
      .toHaveCount(2);
    await expect(page.locator("#citation-section"))
      .toContainText("@I1@ INDI / BIRT");
  });

  test("sélectionne la destination de défilement des citations", async ({ page }) => {
    // Given
    await expect(page.locator("#scroll-citations")).not.toBeChecked();

    // When
    await page.locator("#scroll-citations").check();

    // Then
    await expect(page.locator("#scroll-citations")).toBeChecked();
  });

  test("reste utilisable quand le navigateur bloque localStorage", async ({ page }) => {
    // Given
    await page.addInitScript(() => {
      Object.defineProperty(window, "localStorage", {
        configurable: true,
        get() {
          throw new DOMException("Storage disabled", "SecurityError");
        },
      });
    });

    // When
    await page.reload();

    // Then
    await expect(page.locator(".source-title")).toHaveText("@S1@");

    // When
    await page.locator("#source-filter").fill("cagouille futee");

    // Then
    await expect(page.locator(".source-title")).toHaveText("@S2@");

    // When
    await page.locator("#keep-page-state").uncheck();

    // Then
    await expect(page.locator(".source-title")).toHaveText("@S2@");
  });
});