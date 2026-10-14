(() => {
    "use strict";

    const data = JSON.parse(document.getElementById("report-data").textContent);
    const navigation = document.getElementById("source-navigation");
    const sourcePage = document.getElementById("source-page");
    const reportMain = document.querySelector(".report-main");
    const unmatchedSection = document.getElementById("unmatched-citations");
    const searchInput = document.getElementById("source-filter");
    const previousButton = document.getElementById("previous-source");
    const nextButton = document.getElementById("next-source");
    const firstButton = document.getElementById("first-source");
    const lastButton = document.getElementById("last-source");
    const jumpBackButton = document.getElementById("jump-back");
    const jumpForwardButton = document.getElementById("jump-forward");
    const pageStepSelect = document.getElementById("page-step");
    const scrollTargetRadios = document.querySelectorAll('input[name="section-scroll-target"]');
    const pagePosition = document.getElementById("page-position");
    const sourceCount = document.getElementById("source-count");
    const reportSummary = document.getElementById("report-summary");
    const keepPageState = document.getElementById("keep-page-state");
    const refreshReportButton = document.getElementById("refresh-report");
    const REPORT_STATE_KEY = "gedcom-report-current-source";
    const sortedSources = data.sources.slice().sort(compareSourceReferences);
    let visibleSources = sortedSources.slice();
    let selectedReference = visibleSources.length > 0 ? visibleSources[0].reference : null;
    let selectedScrollTarget = null;

    function getReportStorage() {
        try {
            return window.localStorage;
        } catch (error) {
            return null;
        }
    }

    function removeSavedReference(storage) {
        if (!storage) return;
        try {
            storage.removeItem(REPORT_STATE_KEY);
        } catch (error) {
            // Storage can be disabled by the browser or its privacy settings.
        }
    }

    function loadSavedReference() {
        const storage = getReportStorage();
        if (!keepPageState || !keepPageState.checked || !storage) {
            return;
        }

        try {
            const raw = storage.getItem(REPORT_STATE_KEY);
            if (!raw) {
                return;
            }
            const saved = JSON.parse(raw);
            if (saved && typeof saved.reference === "string" &&
                sortedSources.some((source) => source.reference === saved.reference)) {
                selectedReference = saved.reference;
            }
        } catch (error) {
            removeSavedReference(storage);
        }
    }

    function persistSelectedReference() {
        const storage = getReportStorage();
        if (!storage) return;
        if (!keepPageState || !keepPageState.checked) {
            removeSavedReference(storage);
            return;
        }
        if (!selectedReference) {
            removeSavedReference(storage);
            return;
        }
        try {
            storage.setItem(REPORT_STATE_KEY, JSON.stringify({ reference: selectedReference }));
        } catch (error) {
            // Ignore storage failures silently to keep the UI usable.
        }
    }

    function countLabel(count, singular, plural) {
        return `${count} ${count === 1 ? singular : plural}`;
    }

    function normalizeFilterText(value) {
        return value.normalize("NFD").replace(/\p{Diacritic}/gu, "").toLocaleLowerCase("fr");
    }

    function compareSourceReferences(left, right) {
        const pattern = /^@([A-Za-z])(\d+)@$/;
        const leftMatch = (left.reference || "").match(pattern);
        const rightMatch = (right.reference || "").match(pattern);

        if (!leftMatch || !rightMatch) {
            return (left.reference || "").localeCompare(right.reference || "", "fr");
        }

        const letterOrder = leftMatch[1].toUpperCase().localeCompare(rightMatch[1].toUpperCase());
        if (letterOrder !== 0) return letterOrder;

        const leftNumber = leftMatch[2].replace(/^0+(?=\d)/, "");
        const rightNumber = rightMatch[2].replace(/^0+(?=\d)/, "");
        if (leftNumber.length !== rightNumber.length) return leftNumber.length - rightNumber.length;
        const numberOrder = leftNumber.localeCompare(rightNumber);
        return numberOrder || (left.reference || "").localeCompare(right.reference || "", "fr");
    }

    function element(tagName, className, text) {
        const node = document.createElement(tagName);
        if (className) node.className = className;
        if (text !== undefined) node.textContent = text;
        return node;
    }

    function appendInlineTokens(node, value) {
        const pattern = /https?:\/\/[^\s<>"']+|@[^@\s]+@/gi;
        let cursor = 0;
        let match;

        while ((match = pattern.exec(value)) !== null) {
            if (match.index > cursor) {
                node.append(document.createTextNode(value.slice(cursor, match.index)));
            }
            if (match[0].startsWith("@")) {
                node.append(element("span", "gedcom-pointer", match[0]));
            } else {
                const punctuation = match[0].match(/[.,!?;:]+$/)?.[0] || "";
                const urlText = match[0].slice(0, match[0].length - punctuation.length);
                try {
                    const url = new URL(urlText);
                    if (url.protocol !== "http:" && url.protocol !== "https:") {
                        throw new Error("Unsupported URL protocol");
                    }
                    const link = element("a", "gedcom-link", urlText);
                    link.href = url.href;
                    link.target = "_blank";
                    link.rel = "noopener noreferrer";
                    node.append(link);
                } catch (error) {
                    node.append(document.createTextNode(urlText));
                }
                if (punctuation) {
                    node.append(document.createTextNode(punctuation));
                }
            }
            cursor = match.index + match[0].length;
        }
        if (cursor < value.length) {
            node.append(document.createTextNode(value.slice(cursor)));
        }
    }

    function renderGedcom(text) {
        const pre = element("pre", "gedcom-code");
        const lines = (text || "").replace(/\r/g, "").split("\n");

        lines.forEach((rawLine) => {
            if (rawLine.length === 0) return;

            const match = rawLine.match(/^(\d+)(\s+)(?:(@\S+@)(\s+))?(\S+)(?:\s+(.*))?$/);
            const line = element("span", "gedcom-line");
            const level = match ? Number(match[1]) : 0;
            line.style.paddingInlineStart = `${Math.min(level, 14) * 0.9}rem`;

            if (!match) {
                line.textContent = rawLine;
            } else {
                line.append(element("span", "gedcom-level", match[1]));
                line.append(document.createTextNode(" "));
                if (match[3]) {
                    line.append(element("span", "gedcom-pointer", match[3]));
                    line.append(document.createTextNode(" "));
                }
                line.append(element("span", "gedcom-tag", match[5]));
                if (match[6]) {
                    line.append(document.createTextNode(" "));
                    const value = element("span", "gedcom-value");
                    appendInlineTokens(value, match[6]);
                    line.append(value);
                }
            }
            line.append(document.createTextNode("\n"));
            pre.append(line);
        });

        return pre;
    }

    function excerptBlock(title, text) {
        const details = element("details", "excerpt-block");
        const lines = (text || "").split("\n").filter((line) => line.length > 0).length;
        const summary = element("summary", "excerpt-summary");
        summary.append(element("span", "excerpt-title", title));
        summary.append(element("span", "excerpt-count", `${lines} lignes`));
        details.append(summary);
        details.append(renderGedcom(text));
        details.open = true;
        return details;
    }

    function citationCountFor(source) {
        return data.citations.filter((citation) => citation.reference === source.reference).length;
    }

    function repositoryCountFor(source) {
        return data.repositoryLinks.filter((link) => link.sourceReference === source.reference).length;
    }

    function renderNavigation() {
        const term = normalizeFilterText(searchInput.value.trim());
        visibleSources = sortedSources.filter((source) => {
            const text = normalizeFilterText(`${source.reference || ""} ${source.title || ""} ${source.description || ""}`);
            return text.includes(term);
        });

        if (!visibleSources.some((source) => source.reference === selectedReference)) {
            selectedReference = visibleSources.length > 0 ? visibleSources[0].reference : null;
        }

        navigation.replaceChildren();
        visibleSources.forEach((source, index) => {
            const button = element("button", "source-nav-item");
            button.type = "button";
            button.setAttribute("aria-current", source.reference === selectedReference ? "page" : "false");
            const heading = element("span", "source-nav-heading");
            heading.append(element("span", "source-nav-reference", source.reference || "Sans référence"));
            if (source.title) {
                heading.append(element("span", "source-nav-title", source.title));
            }
            button.append(heading);
            button.append(element("span", "source-nav-meta",
                `${countLabel(repositoryCountFor(source), "dépôt", "dépôts")} · ${countLabel(citationCountFor(source), "citation", "citations")}`));
            button.addEventListener("click", () => {
                selectedReference = source.reference;
                render();
            });
            button.dataset.index = String(index);
            navigation.append(button);
        });

        sourceCount.textContent = `${visibleSources.length} / ${data.sources.length}`;
    }

    function renderSource(source) {
        sourcePage.replaceChildren();
        if (!source) {
            sourcePage.append(element("p", "empty-state", "Aucune source ne correspond au filtre."));
            return;
        }

        const heading = element("header", "source-heading");
        heading.append(element("p", "eyebrow", "ENTITÉ SOURCE"));
        heading.append(element("h2", "source-title", source.reference || "Source sans référence"));
        if (source.title) {
            heading.append(element("p", "source-description", source.title));
        } else if (source.description && source.description !== source.reference) {
            heading.append(element("p", "source-description", source.description));
        }
        sourcePage.append(heading);
        sourcePage.append(excerptBlock("Extrait GEDCOM source", source.gedcom));

        const repositorySection = element("section", "related-section");
        repositorySection.id = "repository-section";
        repositorySection.append(element("h3", "section-heading", "Dépôts associés"));
        const repositoryLinks = data.repositoryLinks.filter((link) => link.sourceReference === source.reference);
        if (repositoryLinks.length === 0) {
            repositorySection.append(element("p", "empty-inline", "Aucun dépôt associé."));
        }
        repositoryLinks.forEach((link) => {
            const repository = data.repositories.find((item) => item.reference === link.repositoryReference);
            const entry = element("article", "related-entry");
            const title = repository ? repository.name : "Dépôt non défini";
            entry.append(element("h4", "related-title", `${link.repositoryReference} · ${title}`));
            entry.append(element("p", "relationship-context", `Contexte : ${link.context}`));
            if (repository) entry.append(excerptBlock("Extrait GEDCOM dépôt", repository.gedcom));
            repositorySection.append(entry);
        });
        sourcePage.append(repositorySection);

        const citationSection = element("section", "related-section");
        citationSection.id = "citation-section";
        citationSection.append(element("h3", "section-heading", "Citations vers cette source"));
        const citations = data.citations.filter((citation) => citation.reference === source.reference);
        if (citations.length === 0) {
            citationSection.append(element("p", "empty-inline", "Aucune citation ne référence cette source."));
        }
        citations.forEach((citation) => {
            const entry = element("article", "related-entry");
            entry.append(element("h4", "related-title", citation.reference || "Citation sans référence"));
            entry.append(element("p", "relationship-context", `Contexte : ${citation.context || "inconnu"}`));
            entry.append(excerptBlock("Extrait GEDCOM citation", citation.gedcom));
            citationSection.append(entry);
        });
        sourcePage.append(citationSection);
        sourcePage.append(element("div", "scroll-tail"));
    }

    function renderUnmatchedCitations() {
        unmatchedSection.replaceChildren();
        const unmatched = data.citations.filter((citation) =>
            !data.sources.some((source) => source.reference === citation.reference));
        if (unmatched.length === 0) return;

        unmatchedSection.append(element("h2", "section-heading", "Citations sans source correspondante"));
        const details = element("details", "excerpt-block unmatched-citations");
        details.append(element("summary", "excerpt-summary", countLabel(unmatched.length, "citation", "citations")));
        unmatched.forEach((citation) => {
            const entry = element("article", "related-entry unmatched-entry");
            entry.append(element("h3", "related-title", citation.reference || "Citation sans référence"));
            entry.append(element("p", "relationship-context", `Contexte : ${citation.context || "inconnu"}`));
            entry.append(excerptBlock("Extrait GEDCOM citation", citation.gedcom));
            details.append(entry);
        });
        unmatchedSection.append(details);
    }

    function render() {
        renderNavigation();
        const selectedIndex = visibleSources.findIndex((source) => source.reference === selectedReference);
        pagePosition.textContent = selectedIndex < 0 ? "0 / 0" : `${selectedIndex + 1} / ${visibleSources.length}`;
        firstButton.disabled = selectedIndex <= 0;
        previousButton.disabled = selectedIndex <= 0;
        nextButton.disabled = selectedIndex < 0 || selectedIndex >= visibleSources.length - 1;
        lastButton.disabled = selectedIndex < 0 || selectedIndex >= visibleSources.length - 1;
        jumpBackButton.disabled = selectedIndex <= 0;
        jumpForwardButton.disabled = selectedIndex < 0 || selectedIndex >= visibleSources.length - 1;
        const pageStep = Number(pageStepSelect.value);
        jumpBackButton.textContent = "«";
        jumpForwardButton.textContent = "»";
        jumpBackButton.setAttribute("aria-label", `Page précédente de ${pageStep} sources`);
        jumpForwardButton.setAttribute("aria-label", `Page suivante de ${pageStep} sources`);
        jumpBackButton.title = `Page précédente de ${pageStep} sources`;
        jumpForwardButton.title = `Page suivante de ${pageStep} sources`;
        renderSource(visibleSources[selectedIndex]);
        reportMain.scrollTop = 0;
        if (selectedScrollTarget) scrollToSection(selectedScrollTarget);

        navigation.querySelectorAll(".source-nav-item").forEach((button) => {
            button.setAttribute("aria-current", button.dataset.index === String(selectedIndex) ? "page" : "false");
        });

        const selectedButton = navigation.querySelector('[aria-current="page"]');
        if (selectedButton) {
            const navigationBounds = navigation.getBoundingClientRect();
            const selectedBounds = selectedButton.getBoundingClientRect();
            if (selectedBounds.top < navigationBounds.top) {
                navigation.scrollTop -= navigationBounds.top - selectedBounds.top;
            } else if (selectedBounds.bottom > navigationBounds.bottom) {
                navigation.scrollTop += selectedBounds.bottom - navigationBounds.bottom;
            }
        }

        persistSelectedReference();
    }

    function scrollToSection(sectionId) {
        const section = document.getElementById(sectionId);
        const heading = section && section.querySelector(".section-heading");
        if (!heading) return;

        const scrollTail = sourcePage.querySelector(".scroll-tail");
        if (scrollTail) scrollTail.style.height = "0px";
        const controlsHeight = document.querySelector(".page-controls").getBoundingClientRect().height;
        if (window.getComputedStyle(reportMain).overflowY === "visible") {
            const delta = heading.getBoundingClientRect().top - controlsHeight - 8;
            const targetScrollTop = window.scrollY + delta;
            const maxScrollTop = document.documentElement.scrollHeight - window.innerHeight;
            if (scrollTail && targetScrollTop > maxScrollTop) {
                scrollTail.style.height = `${targetScrollTop - maxScrollTop}px`;
            }
            window.scrollBy(0, delta);
        } else {
            const panePadding = parseFloat(window.getComputedStyle(reportMain).paddingTop);
            const delta = heading.getBoundingClientRect().top - reportMain.getBoundingClientRect().top - panePadding - controlsHeight - 8;
            const targetScrollTop = reportMain.scrollTop + delta;
            const maxScrollTop = reportMain.scrollHeight - reportMain.clientHeight;
            if (scrollTail && targetScrollTop > maxScrollTop) {
                scrollTail.style.height = `${targetScrollTop - maxScrollTop}px`;
            }
            reportMain.scrollTop = targetScrollTop;
        }
    }

    loadSavedReference();
    reportSummary.textContent = `${countLabel(data.sources.length, "source", "sources")} · ${countLabel(data.repositories.length, "dépôt", "dépôts")} · ${countLabel(data.citations.length, "citation", "citations")}`;
    renderUnmatchedCitations();
    searchInput.addEventListener("input", render);
    pageStepSelect.addEventListener("change", render);
    scrollTargetRadios.forEach((radio) => {
        radio.addEventListener("change", () => {
            if (!radio.checked) return;
            selectedScrollTarget = radio.value || null;
            if (selectedScrollTarget) scrollToSection(selectedScrollTarget);
        });
    });
    keepPageState.addEventListener("change", () => {
        if (!keepPageState.checked) {
            removeSavedReference(getReportStorage());
            return;
        }
        persistSelectedReference();
    });
    refreshReportButton.addEventListener("click", () => {
        persistSelectedReference();
        window.location.reload();
    });
    window.addEventListener("beforeunload", persistSelectedReference);
    firstButton.addEventListener("click", () => {
        if (visibleSources.length > 0) {
            selectedReference = visibleSources[0].reference;
            render();
        }
    });
    previousButton.addEventListener("click", () => {
        const index = visibleSources.findIndex((source) => source.reference === selectedReference);
        if (index > 0) {
            selectedReference = visibleSources[index - 1].reference;
            render();
        }
    });
    nextButton.addEventListener("click", () => {
        const index = visibleSources.findIndex((source) => source.reference === selectedReference);
        if (index >= 0 && index < visibleSources.length - 1) {
            selectedReference = visibleSources[index + 1].reference;
            render();
        }
    });
    lastButton.addEventListener("click", () => {
        if (visibleSources.length > 0) {
            selectedReference = visibleSources[visibleSources.length - 1].reference;
            render();
        }
    });
    jumpBackButton.addEventListener("click", () => {
        const index = visibleSources.findIndex((source) => source.reference === selectedReference);
        if (index >= 0) {
            selectedReference = visibleSources[Math.max(0, index - Number(pageStepSelect.value))].reference;
            render();
        }
    });
    jumpForwardButton.addEventListener("click", () => {
        const index = visibleSources.findIndex((source) => source.reference === selectedReference);
        if (index >= 0) {
            const targetIndex = Math.min(visibleSources.length - 1, index + Number(pageStepSelect.value));
            selectedReference = visibleSources[targetIndex].reference;
            render();
        }
    });
    document.addEventListener("keydown", (event) => {
        if (event.target instanceof HTMLInputElement || event.target instanceof HTMLTextAreaElement) return;
        if (event.key === "ArrowLeft" && !previousButton.disabled) previousButton.click();
        if (event.key === "ArrowRight" && !nextButton.disabled) nextButton.click();
    });
    render();
})();