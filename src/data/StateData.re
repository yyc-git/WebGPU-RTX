open StateDataType;

open StateType;

let stateData = {state: State.create()};

let getStateData = () => {
  stateData;
};

let getState = () => {
  getStateData().state;
};

let setState = state => {
  getStateData().state = state;

  state;
};
