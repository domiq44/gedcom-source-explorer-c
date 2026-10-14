const { defineConfig } = require("@playwright/test");

const port = process.env.PLAYWRIGHT_PORT || "4173";
const baseURL = `http://127.0.0.1:${port}`;

module.exports = defineConfig({
  testDir: "./tests/browser",
  reporter: "list",
  use: {
    baseURL,
    headless: true,
  },
  webServer: {
    command: `python3 -m http.server ${port} --bind 127.0.0.1 --directory build/browser-test`,
    url: baseURL,
    reuseExistingServer: false,
    timeout: 10000,
  },
});