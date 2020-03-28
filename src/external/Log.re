let fatal = message => {
  Js.Exn.raiseError(message);
};

let buildFatalMessage = (~title, ~description, ~reason, ~solution, ~params) => {j|
  Fatal:
  title
  $title
  description
  $description
  reason
  $reason
  solution
  $solution
  params
  $params
   |j};
