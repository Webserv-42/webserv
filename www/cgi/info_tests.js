
const testsInfo = {
    1: 'Prueba',
    2: 'Prueba2',
    3:  'Prueba',
    4:'Prueba',
    5:'Prueba',
    6:'Prueba',
    7:'Prueba',
    8:'Prueba',
    9:'Prueba',
    10:'Prueba',
    11:'Prueba',
    12:'Prueba',
    13:'Prueba',
    14:'Prueba',
    15:'Prueba',
    16:'Prueba',
    17:'Prueba',
}

function openModal(id) {
    // Buscamos en el array global 'tests' (que vive en el otro archivo)
    const test = tests.find(t => t.id === id);
    if (test) {
        document.getElementById('modal-title').textContent = test.title;
        // Buscamos la info extendida en nuestro objeto local
        document.getElementById('modal-desc').textContent = testsInfo[id] || 'Sin información técnica adicional.';
        document.getElementById('modal-overlay').style.display = 'flex';
    }
}

function closeModal() {
    document.getElementById('modal-overlay').style.display = 'none';
}

window.onclick = function(event) {
    const overlay = document.getElementById('modal-overlay');
    if (event.target === overlay) closeModal();
}