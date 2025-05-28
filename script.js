function fetchScoreboard() {
  fetch("scoreboard.txt")
    .then(response => response.text())
    .then(text => {
      const lines = text.split('\n').filter(line => line.trim() !== '');
      const scoreTable = document.getElementById("scoreTable");
      const totalScore = document.getElementById("totalScore");

      scoreTable.innerHTML = "";
      lines.forEach(line => {
        if (line.startsWith("Player") || line.startsWith("---") || line.trim() === "") return;

        if (line.startsWith("Total:")) {
          totalScore.textContent = line;
          return;
        }

        const [name, runs, balls, fours, sixes, status] = line.split('\t');
        const row = `<tr>
          <td>${name}</td>
          <td>${runs}</td>
          <td>${balls}</td>
          <td>${fours}</td>
          <td>${sixes}</td>
          <td>${status}</td>
        </tr>`;
        scoreTable.innerHTML += row;
      });
    });
}

// Refresh scoreboard every 5 seconds
setInterval(fetchScoreboard, 5000);
window.onload = fetchScoreboard;
