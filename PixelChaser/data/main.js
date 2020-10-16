const button = document.querySelector("button");
const isTouchDevice = "ontouchstart" in document.documentElement;

if (isTouchDevice) {

    button.addEventListener("touchstart", () => {
        fetch("button-press");
    }, {
        capture: false,
        passive: true
    });

    button.addEventListener("touchend", () => {
        fetch("button-release");
    }, {
        capture: false,
        passive: true
    });

}
else {

    button.addEventListener("mousedown", () => {
        fetch("button-press");
    }, {
        capture: false,
        passive: true
    });

    button.addEventListener("mouseup", () => {
        fetch("button-release");
    }, {
        capture: false,
        passive: true
    });

}
