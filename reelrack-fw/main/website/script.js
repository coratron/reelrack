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
let data = [
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

function setupModal(button, modal, reelId) {
  // Get the <span> element that closes the modal
  var span = modal.getElementsByClassName("close")[0];

  // Get the input field for ID
  var idField = modal.querySelector("#id");

  // Get other input fields
  var valueField = modal.querySelector("#value");
  var packageField = modal.querySelector("#package");
  var quantityField = modal.querySelector("#quantity");
  var manufacturerField = modal.querySelector("#manufacturer");
  var partNumberField = modal.querySelector("#part_number");
  var skuField = modal.querySelector("#sku");
  var compTypeField = modal.querySelector("#comp_type");

  // When the user clicks the button, open the modal
  button.onclick = function () {
    modal.style.display = "block";
    // Check if reelId is a number
    if (typeof reelId === "number") {
      // Set the reelId field as plain text read-only
      idField.setAttribute("readonly", true);
      idField.style.backgroundColor = "lightgray";
      idField.value = reelId;

      var reelData = data.find(item => item.id === reelId);

    if (reelData) {
      // Set the reelId field as plain text read-only
      idField.setAttribute("readonly", true);
      idField.style.backgroundColor = "lightgray";
      idField.value = reelData.id;

      // Populate other fields using data
      valueField.value = reelData.value;
      packageField.value = reelData.package;
      quantityField.value = reelData.quantity;
      manufacturerField.value = reelData.manufacturer;
      partNumberField.value = reelData.part_number;
      skuField.value = reelData.sku;
      compTypeField.value = reelData.comp_type;
    } 
    } else {
      // Set the reelId field as editable
      idField.removeAttribute("readonly");
      idField.style.backgroundColor = "white";
      idField.value = "";

      //flush all the other values
      valueField.value = "";
      packageField.value = "";
      quantityField.value = "";
      manufacturerField.value = "";
      partNumberField.value = "";
      skuField.value = "";
      compTypeField.value = "";
    }
  };

  // When the user clicks on <span> (x), close the modal
  span.onclick = function () {
    modal.style.display = "none";
  };

  // When the user clicks anywhere outside of the modal, close it
  window.onclick = function (event) {
    if (event.target == modal) {
      modal.style.display = "none";
    }
  };
}

var modal = document.getElementById("myModal");
var btn = document.getElementById("addRow");
setupModal(btn, modal, null);

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
      .forEach((text, index) => {
        const td = document.createElement("td");
        if (index === 0) {
          const reelIDButton = document.createElement("button");
          reelIDButton.textContent = text;
          reelIDButton.addEventListener("click", () => {
            // Trigger the endpoint here
            fetch("http://example.com/endpoint", {
              method: "POST",
              body: JSON.stringify({ reelID: text }),
              headers: {
                "Content-Type": "application/json",
              },
            })
              .then((response) => response.json())
              .then((data) => {
                // Handle the response data here
              })
              .catch((error) => {
                // Handle any errors here
              });
          });
          td.appendChild(reelIDButton);

          const editButton = document.createElement("button");
          editButton.textContent = "✎"; // Edit symbol
          editButton.style.marginLeft = "5px";
          setupModal(editButton, modal, reel.id);
          td.appendChild(editButton);
        } else {
          td.textContent = text;
        }
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

document.getElementById("addRow").addEventListener("click", function () {
  var newRowData = {
    // Data for the new row goes here
  };

  fetch("http://esp32-server/add-row", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(newRowData),
  });
});

// Handle form submission
document
  .getElementById("newRowForm")
  .addEventListener("submit", function (event) {
    event.preventDefault();

    var newRowData = {
      id: Number(document.getElementById("id").value),
      value: document.getElementById("value").value,
      package: document.getElementById("package").value,
      part_number: document.getElementById("part_number").value,
      comp_type: document.getElementById("comp_type").value,
      sku: document.getElementById("sku").value,
      manufacturer: document.getElementById("manufacturer").value,
      quantity: Number(document.getElementById("quantity").value),
    };

    // Find the corresponding object in the data array
    var reelData = data.find(item => item.id === newRowData.id);

    if (reelData) {
      // Update the object with the new values
      Object.assign(reelData, newRowData);
    } else {
      // If the object doesn't exist, add a new one
      data.push(newRowData);
    }

    //FIXME: add again when the server is ready
    // fetch("http://esp32-server/add-row", {
    //   method: "POST",
    //   headers: {
    //     "Content-Type": "application/json",
    //   },
    //   body: JSON.stringify(newRowData),
    // }).then(function () {
    //   // Close the modal
    //   modal.style.display = "none";

    //   // Clear the form
    //   document.getElementById("id").value = "";
    //   document.getElementById("value").value = "";
    //   document.getElementById("package").value = "";
    //   document.getElementById("part_number").value = "";
    //   document.getElementById("comp_type").value = "";
    //   document.getElementById("sku").value = "";
    //   document.getElementById("manufacturer").value = "";
    //   document.getElementById("quantity").value = "";

    //   // Update the table
    //   updateTable();
    // });
    modal.style.display = "none";
    updateTable();
  });
