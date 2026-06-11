/* WRITTEN USING CLAUDE AS I SUCKKK AT UI
 MIGHT DO IT MYSELF IN THE FUTURE
*/

const canvas = document.getElementById('bg');
const ctx = canvas.getContext('2d');
let lines = [];

function resize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    lines = [];
    const rowCount = 14;
    const rowH = canvas.height / rowCount;
    for (let i = 0; i < rowCount; i++) {
        lines.push({
            type: 'h',
            y: rowH * i + rowH * 0.5,
            speed: (0.5 + Math.random() * 0.5) * (i % 2 === 0 ? 1 : -1),
            offset: Math.random() * 100,
            size: 24
        });
    }
}

function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const t = Date.now() / 1000;

    for (const L of lines) {
        ctx.font = `${L.size}px "Space Mono",monospace`;
        ctx.fillStyle = `rgba(0,255,200,0.12)`;
        const charW = L.size * 0.65;
        const shift = ((L.offset + t * L.speed * 35) % charW + charW) % charW;
        const count = Math.floor(canvas.width / charW) + 2;
        const startX = L.speed > 0 ? -charW + shift : -shift;
        for (let c = 0; c < count; c++) {
            ctx.fillText(c % 2 ? '1' : '0', startX + c * charW, L.y);
        }
    }
    requestAnimationFrame(draw);
}

window.addEventListener('resize', resize);
resize();
draw();

function goStage2() {
    document.getElementById('s1').classList.remove('active');
    document.getElementById('s2').classList.add('active');
    let p = 0;
    const iv = setInterval(() => {
        p = Math.min(p + Math.random() * 3, 100);
        document.getElementById('pct').textContent = Math.floor(p) + '%';
        if (p >= 100) { clearInterval(iv); setTimeout(() => goStage3(true, []), 600) }
    }, 80);
}

function goStage3(success, errors) {
    document.getElementById('s2').classList.remove('active');
    document.getElementById('s3').classList.add('active');
    const icon = document.getElementById('icon');
    if (!success) { icon.textContent = '✗'; icon.style.color = '#ff4040'; }
    else if (errors.length) { icon.textContent = '⚠'; icon.style.color = '#ffaa00'; }
    else { icon.textContent = '✓'; icon.style.color = '#00ffc8'; }
    if (errors.length) {
        const log = document.getElementById('log');
        log.style.display = 'flex';
        log.innerHTML = errors.map(e => `<div class="log-entry">▸ ${e}</div>`).join('');
    }
}

function goStage1() {
    document.getElementById('s3').classList.remove('active');
    document.getElementById('s1').classList.add('active');
    document.getElementById('log').style.display = 'none';
    document.getElementById('log').innerHTML = '';
    document.getElementById('pct').textContent = '0%';
}

function setMode(m) {
    document.getElementById('btn-copy').classList.toggle('active', m === 'copy');
    document.getElementById('btn-move').classList.toggle('active', m === 'move');
    document.getElementById('mode-input').value = m;
}