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

  test("navigue entre les sources et filtre sans tenir compte des accents", async ({ page }) => {
    await expect(page.locator("#page-position")).toHaveText("1 / 4");
    await expect(page.locator(".source-title")).toHaveText("@S1@");

    await page.getByRole("button", { name: "Source suivante" }).click();
    await expect(page.locator("#page-position")).toHaveText("2 / 4");
    await expect(page.locator(".source-title")).toHaveText("@S2@");

    await page.locator("#source-filter").fill("cagouille futee");
    await expect(page.locator("#source-count")).toHaveText("1 / 4");
    await expect(page.locator("#source-navigation .source-nav-item"))
      .toHaveCount(1);
    await expect(page.locator(".source-title")).toHaveText("@S2@");

    await page.locator("#source-filter").fill("");
    await page.getByRole("button", { name: "Dernière source" }).click();
    await expect(page.locator("#page-position")).toHaveText("4 / 4");
    await expect(page.locator(".source-title")).toHaveText("@S4@");
  });

  test("affiche les dépôts et citations associés à une source", async ({ page }) => {
    await expect(page.locator("#repository-section"))
      .toContainText("@R1@ · Archives Municipales");
    await expect(page.locator("#citation-section .related-entry"))
      .toHaveCount(2);
    await expect(page.locator("#citation-section"))
      .toContainText("@I1@ INDI / BIRT");

    await page.locator("#scroll-citations").check();
    await expect(page.locator("#scroll-citations")).toBeChecked();
  });

  test("reste utilisable quand le navigateur bloque localStorage", async ({ page }) => {
    await page.addInitScript(() => {
      Object.defineProperty(window, "localStorage", {
        configurable: true,
        get() {
          throw new DOMException("Storage disabled", "SecurityError");
        },
      });
    });
    await page.reload();

    await expect(page.locator(".source-title")).toHaveText("@S1@");
    await page.locator("#source-filter").fill("cagouille futee");
    await expect(page.locator(".source-title")).toHaveText("@S2@");
    await page.locator("#keep-page-state").uncheck();
    await expect(page.locator(".source-title")).toHaveText("@S2@");
  });
});