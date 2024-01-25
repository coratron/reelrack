// Fetch the reel data from your server
fetch("http://your-server-url/api/reels")
  .then((response) => response.json())
  .then((data) => {
    // Get the div where we will put the reel data
    const reelDataDiv = document.getElementById("reelData");

    // Create a table
    const table = document.createElement("table");

    // Create table header
    const thead = document.createElement("thead");
    const headerRow = document.createElement("tr");
    [
      "ID",
      "Value",
      "Package",
      "Part Number",
      "Component Type",
      "SKU",
      "Manufacturer",
      "Quantity",
    ].forEach((text) => {
      const th = document.createElement("th");
      th.textContent = text;
      headerRow.appendChild(th);
    });
    thead.appendChild(headerRow);
    table.appendChild(thead);

    // Create table body
    const tbody = document.createElement("tbody");
    data.forEach((reel) => {
      const row = document.createElement("tr");
      [
        reel.id,
        reel.value,
        reel.package,
        reel.part_number,
        reel.comp_type,
        reel.sku,
        reel.manufacturer,
        reel.quantity,
      ].forEach((text) => {
        const td = document.createElement("td");
        td.textContent = text;
        row.appendChild(td);
      });
      tbody.appendChild(row);
    });
    table.appendChild(tbody);

    // Add the table to the div
    reelDataDiv.appendChild(table);
  })
  .catch((error) => console.error("Error:", error));

// Mock data
const data = [
  {
    id: 0,
    value: "Value 1",
    package: "Package 1",
    quantity: 10,
    manufacturer: "Manufacturer 1",
    part_number: "Part Number 1",
    sku: "SKU 1",
    comp_type: "Component Type 1",
  },
  {
    id: 1,
    value: "Value 2",
    package: "Package 2",
    quantity: 20,
    manufacturer: "Manufacturer 2",
    part_number: "Part Number 2",
    sku: "SKU 2",
    comp_type: "Component Type 2",
  },
  // Add more objects as needed
];

// Get the div where we will put the reel data
const reelDataDiv = document.getElementById("reelData");

// Create a table
const table = document.createElement("table");

// Create table header
const thead = document.createElement("thead");
const headerRow = document.createElement("tr");
[
  "ID",
  "Value",
  "Package",
  "Part Number",
  "Component Type",
  "SKU",
  "Manufacturer",
  "Quantity",
].forEach((text) => {
  const th = document.createElement("th");
  th.textContent = text;
  headerRow.appendChild(th);
});
thead.appendChild(headerRow);
table.appendChild(thead);

// Create table body
const tbody = document.createElement("tbody");
data.forEach((reel) => {
  const row = document.createElement("tr");
  [
    reel.id,
    reel.value,
    reel.package,
    reel.part_number,
    reel.comp_type,
    reel.sku,
    reel.manufacturer,
    reel.quantity,
  ].forEach((text) => {
    const td = document.createElement("td");
    td.textContent = text;
    row.appendChild(td);
  });
  tbody.appendChild(row);
});
table.appendChild(tbody);

// Add the table to the div
reelDataDiv.appendChild(table);

// Get the checkboxes
const manufacturerCheckbox = document.getElementById("toggle-manufacturer");
const skuCheckbox = document.getElementById("toggle-sku");
const quantityCheckbox = document.getElementById("toggle-quantity");

// Add event listeners
manufacturerCheckbox.addEventListener("change", updateTable);
skuCheckbox.addEventListener("change", updateTable);
quantityCheckbox.addEventListener("change", updateTable);

function updateTable() {
  // Clear the table
  while (table.firstChild) {
    table.removeChild(table.firstChild);
  }

  // Create table header
  const thead = document.createElement("thead");
  const headerRow = document.createElement("tr");
  [
    "ID",
    "Value",
    "Package",
    "Part Number",
    "Component Type",
    skuCheckbox.checked ? "SKU" : null,
    manufacturerCheckbox.checked ? "Manufacturer" : null,
    quantityCheckbox.checked ? "Quantity" : null,
  ]
    .filter(Boolean)
    .forEach((text) => {
      const th = document.createElement("th");
      th.textContent = text;
      headerRow.appendChild(th);
    });
  thead.appendChild(headerRow);
  table.appendChild(thead);

  // Create table body
  const tbody = document.createElement("tbody");
  data.forEach((reel) => {
    const row = document.createElement("tr");
    [
      reel.id,
      reel.value,
      reel.package,
      reel.part_number,
      reel.comp_type,
      skuCheckbox.checked ? reel.sku : null,
      manufacturerCheckbox.checked ? reel.manufacturer : null,
      quantityCheckbox.checked ? reel.quantity : null,
    ]
      .filter((item) => item !== null)
      .forEach((text) => {
        const td = document.createElement("td");
        td.textContent = text;
        row.appendChild(td);
      });
    tbody.appendChild(row);
  });
  table.appendChild(tbody);
}

// Initial table update
updateTable();

document
  .getElementById("toggle-manufacturer")
  .addEventListener("change", updateTable);
document.getElementById("toggle-sku").addEventListener("change", updateTable);
document
  .getElementById("toggle-quantity")
  .addEventListener("change", updateTable);
