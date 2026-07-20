document.getElementById("log-form").addEventListener("submit", async function(e) {
  e.preventDefault();

  console.log("Form submitted");
  const formData = new FormData(e.target);

  const startStr = formData.get("start");
  const endStr = formData.get("end");

  const start = Math.floor(new Date(startStr).getTime() / 1000);
  const end = Math.floor(new Date(endStr).getTime() / 1000);

  if (isNaN(start)) {
    alert("Invalid start date");
    return;
  }

  const url = new URL("http://localhost:8080/logs");
  url.searchParams.append("start", start);
  if (!isNaN(end)) {
    url.searchParams.append("end", end);
  }
  const resp = await fetch(url.toString());
  const data = await resp.json();
  console.log(data);
});