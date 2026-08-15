const INFO = 0;
const WARNING = 1;
const ERROR = 2;

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
  
  const text = data.map(log => {
    const date = new Date(log.timestamp * 1000);
    const status = log.status === 0 ? "INFO" : log.status === 1 ? "WARNING" : "ERROR";
    const worker = log.worker_id;
    return `[${date.toISOString()}] [Worker ${worker}] ${status}`;
  }).join("\n");
  document.getElementById("log-display").textContent = text;

  const errorCount = data.filter(log => log.status === ERROR).length;
  const warningCount = data.filter(log => log.status === WARNING).length;
  const infoCount = data.filter(log => log.status === INFO).length;

  document.getElementById("total-count").textContent = data.length;
  document.getElementById("error-count").textContent = errorCount;
  document.getElementById("warning-count").textContent = warningCount;
  document.getElementById("info-count").textContent = infoCount;
});
 