// add necessary listeners

const PROGRESS_INTERVAL = 250;

document.getElementById('flux-btn').addEventListener('click', onFlux);
document.getElementById('again-btn').addEventListener('click', onAgain);
window.addEventListener('beforeunload', () => fetch('/quit'));

function formatTimeRemaining(totalSeconds) {
    if (totalSeconds < 0) return "Unknown time remaining";
    if (totalSeconds === 0) return "0Secs";

    const SECONDS_IN_MINUTE = 60;
    const SECONDS_IN_HOUR = 3600;
    const SECONDS_IN_DAY = 86400;

    // Break down the total seconds
    const days = Math.floor(totalSeconds / SECONDS_IN_DAY);
    const hours = Math.floor((totalSeconds % SECONDS_IN_DAY) / SECONDS_IN_HOUR);
    const minutes = Math.floor((totalSeconds % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE);
    const seconds = totalSeconds % SECONDS_IN_MINUTE;

    // Construct the string dynamically so we don't display "0Days" if it's shorterr
    let parts = [];
    if (days > 0) parts.push(`${days}Days`);
    if (hours > 0 || days > 0) parts.push(`${hours}Hours`);
    if (minutes > 0 || hours > 0 || days > 0) parts.push(`${minutes}Mins`);
    parts.push(`${seconds}Secs`);

    return parts.join(' ');
}

// checks the parameters and calls byteflux client
async function onFlux() {
    const src = document.getElementById('src').value;
    const dst = document.getElementById('dst').value;
    const mode = document.getElementById('mode-input').value;

    if (src === "" || dst === "") return;
    ui_stage2();

    // update progress every PROGRESS_INTERVAL ms
    const progress_poll = setInterval(async () => {
        const res = await fetch('/progress');
        const data = await res.json();

        // percentage
        const percent = data.j_percent;
        // MB/s speed
        const bps = parseInt(data.j_speed_bps) || 0;
        const mbps = (bps / (1024 * 1024)).toFixed(1);

        // update time
        const secs = parseInt(data.j_time_remaining_s);
        timerem_string = formatTimeRemaining(secs);

        // update
        ui_set_progress(percent, mbps, timerem_string);

    }, PROGRESS_INTERVAL);

    const result = await fetch(`/transfer?src=${src}&dst=${dst}&mode=${mode}`, { method: 'POST' });
    const data = await result.json();

    ui_stage3(data.j_success, data.j_fatal_error, data.j_file_errors);

    // after ui is changed, remove progress updating
    clearInterval(progress_poll);
}

// if again is clicked set to ui stage 1
async function onAgain() {
    ui_stage1();
}

setInterval(() => fetch('/heartbeat'), 3000);