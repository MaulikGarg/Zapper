// add necessary listeners

const PROGRESS_INTERVAL = 50;

document.getElementById('flux-btn').addEventListener('click', onFlux);
document.getElementById('again-btn').addEventListener('click', onAgain);
window.addEventListener('beforeunload', () => fetch('/quit'));

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
        const percent = await res.text();
        ui_set_progress(percent);
    }, PROGRESS_INTERVAL);
    const speed_poll = setInterval(async () => {
        const res = await fetch('/speed');
        const bps = parseInt(await res.text());
        const mbps = (bps / (1024 * 1024)).toFixed(1);
        ui_set_speed(mbps);
    }, 1000);

    const result = await fetch(`/transfer?src=${src}&dst=${dst}&mode=${mode}`, { method: 'POST' });
    const data = await result.json();

    ui_stage3(data.success, data.fatal_error, data.file_errors);

    // after ui is changed, remove progress updating
    clearInterval(progress_poll);
    clearInterval(speed_poll);
}

// if again is clicked set to ui stage 1
async function onAgain() {
    ui_stage1();
}

setInterval(() => fetch('/heartbeat'), 3000);