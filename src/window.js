window.addEventListener("resize", function() {
    const cont1Height = document.getElementById("cont1").offsetHeight;
    document.getElementById("cont2").style.height = cont1Height + "px";
});

window.addEventListener("DOMContentLoaded", function() {
    setTimeout(function() {
        const cont1Height = document.getElementById("cont1").offsetHeight;
        document.getElementById("cont2").style.height = cont1Height + "px";
    }, 10);
});