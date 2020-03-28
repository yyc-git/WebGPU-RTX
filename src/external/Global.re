type timer;

[@bs.val] external setTimeout: (unit => unit, float) => timer = "setTimeout";
[@bs.val] external clearTimeout: timer => unit = "clearTimeout";
