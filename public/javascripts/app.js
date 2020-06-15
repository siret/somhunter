
/* This file is part of SOMHunter.
 *
 * Copyright (C) 2020 František Mejzlík <frankmejzlik@gmail.com>
 *                    Mirek Kratochvil <exa.exa@gmail.com>
 *                    Patrik Veselý <prtrikvesely@gmail.com>
 *
 * SOMHunter is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * SOMHunter is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SOMHunter. If not, see <https://www.gnu.org/licenses/>.
 */
$(document).foundation();

function showGlobalMessage(messagePrimary, messageSecondary, time, type = "i") {
  let cls = "";
  if (type == "w") {
    cls = "warning";
  } else if (type == "e") {
    cls = "alert";
  } else if (type == "s") {
    cls = "success";
  }

  $mainGlobalMessageCont = $("#mainGlobalMessageCont");

  $mainGlobalMessageCont.removeClass();
  $mainGlobalMessageCont.addClass("callout");
  $mainGlobalMessageCont.addClass(cls);

  $primary = $($mainGlobalMessageCont.children(".primary")[0]);
  $secondary = $($mainGlobalMessageCont.children(".secondary")[0]);

  $mainGlobalMessageCont.show();
  $primary.html(messagePrimary);
  $secondary.html(messageSecondary);

  setTimeout(() => {
    clearGlobalMesssage();
  }, time);
}

function clearGlobalMesssage() {
  $mainGlobalMessageCont = $("#mainGlobalMessageCont");
  $primary = $($mainGlobalMessageCont.children(".primary")[0]);
  $secondary = $($mainGlobalMessageCont.children(".secondary")[0]);

  $primary.html("");
  $secondary.html("");
  $mainGlobalMessageCont.hide();
}

function onDocumentReady(fn) {
  // see if DOM is already available
  if (document.readyState === "complete" || document.readyState === "interactive") {
    // call on next available tick
    setTimeout(fn, 1);
  } else {
    document.addEventListener("DOMContentLoaded", fn);
  }
}

function boldString(str, find) {
  const re = new RegExp(find, "gi");
  return str.replace(re, "<strong>" + find + "</strong>");
}
