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
    console.log(data);
    data.forEach((reel, rowIndex) => {
      let row = document.createElement("tr");
      row.id = `row-${rowIndex}`;
      [
        reel.id,
        reel.value,
        reel.package,
        reel.part_number,
        reel.comp_type,
        reel.sku,
        reel.manufacturer,
        reel.quantity,
      ].forEach((text, columnIndex) => {
        let td = document.createElement("td");
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
let data = [];

for (let i = 0; i < 40; i++) {
  data.push({
    id: i,
    value: `Value ${i + 1}`,
    package: `Package ${i + 1}`,
    quantity: 10 * (i + 1),
    manufacturer: `Manufacturer ${i + 1}`,
    part_number: `Part Number ${i + 1}`,
    sku: `SKU ${i + 1}`,
    comp_type: `Component Type ${i + 1}`,
  });
}

let settings = {
  numRows: 2,
  numReelsPerRow: 35,
  ledColour: "#FF0000",
};

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

/// Create table body
const tbody = document.createElement("tbody");
console.log(data);
data.forEach((reel, rowIndex) => {
  let row = document.createElement("tr");
  row.id = `row-${rowIndex}`;
  console.log(row.id);
  [
    reel.id,
    reel.value,
    reel.package,
    reel.part_number,
    reel.comp_type,
    reel.sku,
    reel.manufacturer,
    reel.quantity,
  ].forEach((text, columnIndex) => {
    let td = document.createElement("td");
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

      var reelData = data.find((item) => item.id === reelId);

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
  data.forEach((reel, rowIndex) => {
    let row = document.createElement("tr");
    row.id = `row-${rowIndex}`;
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
          // reelIDButton.addEventListener("click", () => {
          //   // Trigger the endpoint here
          //   fetch("http://example.com/endpoint", {
          //     method: "POST",
          //     body: JSON.stringify({ reelID: text }),
          //     headers: {
          //       "Content-Type": "application/json",
          //     },
          //   })
          //     .then((response) => response.json())
          //     .then((data) => {
          //       // Handle the response data here
          //     })
          //     .catch((error) => {
          //       // Handle any errors here
          //     });
          // });
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
    var reelData = data.find((item) => item.id === newRowData.id);

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

function configureSettingsModal(modal, button, sform) {
  // Get the <span> element that closes the modal
  var span = modal.getElementsByClassName("close")[0];

  button.onclick = function () {
    modal.style.display = "block";

    //load stored values in settings
    document.getElementById("numRows").value = settings.numRows;
    document.getElementById("numReelsPerRow").value = settings.numReelsPerRow;
    document.getElementById("ledColour").value = settings.ledColour;
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

  sform.addEventListener("submit", function (event) {
    event.preventDefault();

    //check if the values are valid
    if (document.getElementById("numRows").value <= 0) {
      alert("Number of rows must be greater than 0");
      return;
    } else if (document.getElementById("numReelsPerRow").value <= 0) {
      alert("Number of reels per row must be greater than 0");
      return;
    } else {
      settings.numRows = document.getElementById("numRows").value;
      settings.numReelsPerRow = document.getElementById("numReelsPerRow").value;
      settings.ledColour = document.getElementById("ledColour").value;
    }

    //FIXME: send to endpoint
    fetch("http://esp32-server/settings", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(settings),
    }).then(function () {
      // Update the table
      updateTable();
    });

    // Close the modal
    modal.style.display = "none";

    // reload racktable
    loadRackTable();
  });
}

let settingsModal = document.getElementById("settingsModal");
let settingsButton = document.getElementById("config-button");
let settingsForm = document.getElementById("settingsForm");
configureSettingsModal(settingsModal, settingsButton, settingsForm);

function loadRackTable() {
  // Get the rackTable element
  let rackTable = document.getElementById("rackTable");

  // Remove all existing rows from the rackTable
  while (rackTable.firstChild) {
    rackTable.removeChild(rackTable.firstChild);
  }

  // Get the number of rows and columns from your settings
  let numRows = settings.numRows;
  let numColumns = settings.numReelsPerRow;

  // Loop through the number of rows
  for (let i = 0; i < numRows; i++) {
    // Create a new row
    let row = document.createElement("tr");

    // Loop through the number of columns
    for (let j = 0; j < numColumns; j++) {
      // Create a new cell
      let cell = document.createElement("td");

      // Get the reelData div
      let reelDataDiv = document.getElementById("reelData");

      // Get the table inside the reelData div
      let table = reelDataDiv.querySelector("table");

      // Get the rows in the table
      let rows = table.querySelectorAll("tr");

      // Get the number of rows
      let numRows = rows.length - 1;

      // If the corresponding row in the reelData table is missing, add a class to the cell
      if (i * numColumns + j >= numRows) {
        cell.classList.add("missing-row");
      }

      console.log(numRows);

      // Populate the cell with the index number
      cell.innerText = i * numColumns + j;

      // Add an onclick event to the cell
      cell.onclick = function () {
        // Get the id of the cell
        const id = this.innerText;
        console.log(id);

        // Get the corresponding row in the reelData table
        let targetRow = document.getElementById(`row-${id}`);
        console.log(targetRow);

        // Calculate the scroll position
        let scrollPosition = targetRow.offsetTop - window.innerHeight / 2;

        // Scroll to the row
        window.scrollTo({ top: scrollPosition, behavior: "smooth" });

        //clear all the backgrounds for all rows
        for (let i = 0; i < numRows; i++) {
          let row = document.getElementById(`row-${i}`);
          row.style.backgroundColor = "";
        }

        // Change the background color of the row
        targetRow.style.backgroundColor = "lightgreen";

        console.log("Clicked cell " + id);
      };
      // Append the cell to the row
      row.appendChild(cell);
    }

    // Append the row to the table
    rackTable.appendChild(row);
  }
}

//load at least once
loadRackTable();

// Get the search bar
const searchBar = document.getElementById("searchBar");

// Add an input event listener to the search bar
searchBar.addEventListener("input", function () {
  // Get the search string
  const searchString = this.value.toLowerCase();

  // Get the rows in the reelData table
  const rows = document.getElementById("reelData").getElementsByTagName("tr");

  // Loop through the rows
  for (let i = 1; i < rows.length; i++) {
    // Get the cells in the row
    const cells = rows[i].getElementsByTagName("td");

    // Loop through the cells
    let found = false;
    for (let j = 0; j < cells.length && !found; j++) {
      // Check if the cell's text includes the search string
      if (cells[j].textContent.toLowerCase().includes(searchString)) {
        // Show the row
        rows[i].style.display = "";
        found = true;
      }
    }

    // If no cell in the row includes the search string, hide the row
    if (!found) {
      rows[i].style.display = "none";
    }
  }
});
